#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <raylib.h>


typedef struct chip8{
    uint8_t memory[4096]; //RAM
    uint8_t Vx[16]; //General Purpose Registers FROM 0 THROUGH F;
    uint16_t I; // I REGISTER 
    uint16_t PC; // PROGRAM COUNTER REGISTER 
    uint8_t SP; // STACK POINTER REGISTER 
    uint16_t stack[16];
    uint8_t delay; //DELAY TIMER REGISTER
    uint8_t sound; // SOUND TIMER REGISTER
    uint8_t display[32][64];
    uint8_t keymap[16];
} chip8;

void OP_00E0(chip8* context){
    printf("OP: 00E0 (CLS)\n");
    memset(context->display,0,sizeof(uint8_t)*64*32);
}

void OP_00EE(chip8* context){
    printf("OP: 00EE (RETURN FROM SUBROUTINE)\n");
    context->PC=context->stack[--(context->SP)];
}


void OP_1NNN(chip8* context,uint16_t nnn){
    printf("OP: 1NNN (JMP) => %hu\n",nnn);
    context->PC=nnn;
}

void OP_2NNN(chip8* context,uint16_t nnn){
    printf("OP: 2NNN (CALL SUBROUTINE AT %hhu)",nnn);
    context->stack[context->SP++] = context->PC;
    context->PC=nnn;
}

void OP_3XNN(chip8* context,uint8_t x, uint8_t nn){
    printf("OP: 3XNN (SKIP ONE INSTRUCTION IF %hhu == %hhu)",context->Vx[x],nn);
    if(context->Vx[x] == nn)
        context->PC += 2;
}

void OP_4XNN(chip8* context,uint8_t x, uint8_t nn){
    printf("OP: 4XNN (SKIP ONE INSTRUCTION IF %hhu != %hhu)",context->Vx[x],nn);
    if(context->Vx[x] != nn)
        context->PC += 2;
}

void OP_5XY0(chip8* context, uint8_t x, uint8_t y){
    printf("OP: 5XYN (SKIP ONE INSTRUCTION IF %hhu == %hhu)",context->Vx[x],context->Vx[y]);
    if(context->Vx[x] == context->Vx[y])
        context->PC += 2;
}

void OP_9XY0(chip8* context, uint8_t x, uint8_t y){
    printf("OP: 9XYN (SKIP ONE INSTRUCTION IF %hhu != %hhu)",context->Vx[x],context->Vx[y]);
    if(context->Vx[x] != context->Vx[y])
        context->PC += 2;
}

void OP_6XNN(chip8* context, uint8_t x, uint8_t nn){
    printf("OP: 6XNN (SET V%hhu REGISTER) => %hu\n",x,nn);
    context->Vx[x] = nn; 
}

void OP_7XNN(chip8* context, uint8_t x, uint8_t nn){
    printf("OP: 7XNN (ADD  TO V%hhu REGISTER) => REGISTER VALUE (%hu) + %hu\n",x,context->Vx[x],nn);
        context->Vx[x] += nn;
}

void OP_8XY0(chip8* context, uint8_t x, uint8_t y){
    printf("OP: 8XY0 (SET REGISTER VX TO VALUE OF VY) VX %hhu <= VY %hhuì\n",context->Vx[x],context->Vx[y]);
    context->Vx[x] = context->Vx[y];
}

void OP_8XY1(chip8* context, uint8_t x, uint8_t y,uint8_t config){
    printf("OP: 8XY1 (SET REGISTER VX TO VALUE OF VX | VY) VX %hhu <= VY %hhuì\n",context->Vx[x],context->Vx[y]);
    context->Vx[x] = context->Vx[x] | context->Vx[y];
    if(config)
    context->Vx[15] = 0;
}

void OP_8XY2(chip8* context, uint8_t x, uint8_t y,uint8_t config){
    printf("OP: 8XY2 (SET REGISTER VX TO VALUE OF VX & VY) VX %hhu <= VY %hhuì\n",context->Vx[x],context->Vx[y]);
    context->Vx[x] = context->Vx[x] & context->Vx[y];
    if(config)
    context->Vx[15] = 0;
}

void OP_8XY3(chip8* context, uint8_t x, uint8_t y,uint8_t config){
    printf("OP: 8XY3 (SET REGISTER VX TO VALUE OF VX ^ VY) VX %hhu <= VY %hhuì\n",context->Vx[x],context->Vx[y]);
    context->Vx[x] = context->Vx[x] ^ context->Vx[y];
    if(config)
    context->Vx[15] = 0;
}

void OP_8XY4(chip8* context, uint8_t x, uint8_t y){
    uint8_t carry = 0;
    printf("OP: 8XY4 (SET REGISTER VX TO VALUE OF VX + VY) VX %hhu <= VY %hhuì\n",context->Vx[x],context->Vx[y]);
    if(context->Vx[x] + context->Vx[y] > 255)
        carry = 1;
    context->Vx[x] = context->Vx[x] + context->Vx[y];
    context->Vx[15] = carry;
}

void OP_8XY5(chip8* context, uint8_t x, uint8_t y){
    printf("OP: 8XY5 (SET REGISTER VX TO VALUE OF VX - VY) VX %hhu <= VY %hhu\n",context->Vx[x],context->Vx[y]);
    uint8_t carry = 0;
    if(context->Vx[x] >= context->Vx[y])
        carry = 1;
    context->Vx[x] = context->Vx[x] - context->Vx[y];
    context->Vx[15] = carry;
}

void OP_8XY6(chip8* context, uint8_t x, uint8_t y, uint8_t config){
    printf("OP: 8XY4 (SHIFT VX TO THE RIGHT) VX %hhu\n",context->Vx[x]);
    uint8_t carry;
    if(config == 1)
        context->Vx[x] = context->Vx[y];
    carry = 0x01 & context->Vx[x];
    context->Vx[x] >>= 1;
    context->Vx[15] = carry;
}

void OP_8XYE(chip8* context, uint8_t x, uint8_t y, uint8_t config){
    printf("OP: 8XY4 (SHIFT VX TO THE LEFT) VX %hhu\n",context->Vx[x]);
    uint8_t carry;
    if(config == 1)
        context->Vx[x] = context->Vx[y];
    carry = (0x80 & context->Vx[x]) > 0;
    context->Vx[x] <<= 1 ;
    context->Vx[15] = carry;
}

void OP_8XY7(chip8* context, uint8_t x, uint8_t y){
    printf("OP: 8XY7 (SET REGISTER VX TO VALUE OF VY - VX) VX %hhu <= VY %hhuì\n",context->Vx[x],context->Vx[y]);
    uint8_t carry = 0;
    if(context->Vx[x] <= context->Vx[y])
        carry = 1;
    context->Vx[x] = context->Vx[y] - context->Vx[x];
    context->Vx[15] = carry;

}



void OP_ANNN(chip8* context,uint16_t nnn){
    printf("OP: ANNN (SET I REGISTER) => %hu\n",nnn);
    context->I = nnn; 
}

void OP_BNNN(chip8* context, uint16_t nnn, uint8_t x,uint8_t config){
     printf("OP: BNNN (JUMP TO ADDRESS NNN + VALUE IN VX[0]) => %hu\n",nnn);
    if(config == 1){
        context->PC = nnn + context->Vx[0]; //Old functioning
    }
    if(config == 0){
        context->PC = nnn + context->Vx[x]; //new functioning
    }
}

void OP_CXNN(chip8* context, uint8_t nn, uint8_t x){
    printf("OP: CXNN (RANDOM NUMBER IN VX)");
    uint8_t random = rand();
    context->Vx[x] =  random & nn;

}

void OP_DXYN(chip8* context,uint8_t x, uint8_t y, uint8_t n){
    printf("OP: DXYN (DRAW TO SCREEN OFFSET X = %hhu OFFSET Y = %hhu) FIRST SPRITE LOCATION %hu\n", context->Vx[x] % 64,context->Vx[y],context->I);

    uint8_t x_cord = context->Vx[x] % 64;
    uint8_t y_cord = context->Vx[y] % 64;
    context->Vx[15] = 0;
    uint8_t sprite_data;
    uint8_t mask;

        for(uint8_t i = 0; i < n ; i++){
        sprite_data=context->memory[context->I + i];
        x_cord=context->Vx[x] % 64;
        if(y_cord + i > 32)
            break;
        for (mask = 0x80; mask != 0; mask >>= 1) {
            if (sprite_data & mask) {
                if(context->display[y_cord+i][x_cord] == 1){
                    context->display[y_cord+i][x_cord] = 0;
                    context->Vx[15] = 1;
                }
                else if(context->display[y_cord+i][x_cord] == 0)
                        context->display[y_cord+i][x_cord] = 1;            
            }
            x_cord++;
            if(x_cord > 63)
                break;
        }
    } 
}

void OP_EX9E(chip8* context,uint8_t x){
    printf("OP: EX9E (SKIP INSTRUCTION IF KEY IN VX IS PRESSED) KEY: %hu\n", context->Vx[x]);
    if(IsKeyDown(context->keymap[context->Vx[x]]))
        context->PC += 2;

}

void OP_EXA1(chip8* context,uint8_t x){
    printf("OP: EXA1 (SKIP INSTRUCTION IF KEY IN VX IS NOT PRESSED) KEY: %hu\n", context->Vx[x]);
    if(!IsKeyDown(context->keymap[context->Vx[x]]))
        context->PC += 2;

}

void OP_FX07(chip8* context, uint8_t x){
    printf("OP: OP_FX07 (SET VX TO VALUE OF TIMER) VALUE OF TIMER: %hu , X : %hu\n", context->delay, x);
    context->Vx[x] = context->delay;
}

void OP_FX15(chip8* context, uint8_t x){
    printf("OP: OP_FX15 (SET DELAY TIMER TO VALUE OF VX) VALUE OF TIMER: %hu , X : %hu\n", context->delay, x);
    context->delay = context->Vx[x];
}

void OP_FX18(chip8* context, uint8_t x){
    printf("OP: OP_FX18 (SET SOUND TIMER TO VALUE OF VX) VALUE OF TIMER: %hu , X : %hu\n", context->sound, x);
    context->sound = context->Vx[x];
}

void OP_FX1E(chip8* context, uint8_t x){
    printf("OP: OP_FX1E (ADD VALUE OF VX TO I) VALUE OF VX: %hu , I : %hu\n", context->Vx[x], context->I);
    if(context->I + context->Vx[x] >= 4096)
        context->Vx[15] = 1;
    context->I += context->Vx[x];
}

void OP_FX0A(chip8* context, uint8_t x){
    printf("OP: FX0A (WAIT FOR INPUT AND SAVE IT TO VX)");
    uint8_t key = GetKeyPressed();
    if(key == 0)
        context->PC -= 2;
    else
        context->Vx[x] = key;
}

void OP_FX29(chip8* context, uint8_t x){
    printf("OP: FX29 (SET I TO ADDRESS OF CHARACTER IN VX) VX: %#4x",context->Vx[x]);
    if(context->Vx[x] > 0xF)
        printf("ERROR BAD FONT CHARACTER\n");
    else 
        context->I=context->Vx[x] * 5;
}

void OP_FX33(chip8* context, uint8_t x){
    printf("OP: FX33 BINARY DECIMAL CONVERSION) VX: %#4x",context->Vx[x]);
    uint8_t value = context->Vx[x];
    context->memory[context->I] = value/100 % 10;
    context->memory[(context->I)+1] = value/10 % 10;
    context->memory[(context->I)+2] = value % 10;
    printf("%d %d %d", context->memory[context->I], context->memory[context->I+1], context->memory[context->I+2]);
}

void OP_FX55(chip8* context, uint8_t x,uint8_t config){
    printf("OP: FX55 (STORE IN MEMORY AT I FROM V0 TO VX) I: %hu",context->I);
    for(int i = 0; i<=x; i++){
        context->memory[context->I + i] = context->Vx[i];
    }
    if(config == 1)
        context->I += x + 1;
}

void OP_FX65(chip8* context, uint8_t x,uint8_t config){
    printf("OP: FX65 (LOAD FROM MEMORY AT I FROM V0 TO VX) I: %hu",context->I);
    for(int i = 0; i<=x; i++){
        context->Vx[i] = context->memory[context->I + i];
    }
    if(config == 1)
        context->I += x + 1;
}

void loadfonts(chip8* context){

    context->memory[0] = 0xF0;
    context->memory[1] = 0x90;
    context->memory[2] = 0x90;  // "0"
    context->memory[3] = 0x90;
    context->memory[4] = 0xF0;

    context->memory[5] = 0x20;
    context->memory[6] = 0x60;
    context->memory[7] = 0x20; //"1"
    context->memory[8] = 0x20;
    context->memory[9] = 0x70;

    context->memory[10] = 0xF0;
    context->memory[11] = 0x10;
    context->memory[12] = 0xF0; //"2"
    context->memory[13] = 0x80;
    context->memory[14] = 0xF0;

    context->memory[15] = 0xF0;
    context->memory[16] = 0x10;
    context->memory[17] = 0xF0; //"3"
    context->memory[18] = 0x10;
    context->memory[19] = 0xF0;

    context->memory[20] = 0x90;
    context->memory[21] = 0x90;
    context->memory[22] = 0xF0; //"4"
    context->memory[23] = 0x10;
    context->memory[24] = 0x10;

    context->memory[25] = 0xF0;
    context->memory[26] = 0x80;
    context->memory[27] = 0xF0; //"5"
    context->memory[28] = 0x10;
    context->memory[29] = 0xF0;

    context->memory[30] = 0xF0;
    context->memory[31] = 0x80;
    context->memory[32] = 0xF0; //"6"
    context->memory[33] = 0x90;
    context->memory[34] = 0xF0;

    context->memory[35] = 0xF0;
    context->memory[36] = 0x10;
    context->memory[37] = 0x20; //"7"
    context->memory[38] = 0x40;
    context->memory[39] = 0x40;

    context->memory[40] = 0xF0;
    context->memory[41] = 0x90;
    context->memory[42] = 0xF0; //"8"
    context->memory[43] = 0x90;
    context->memory[44] = 0xF0;

    context->memory[45] = 0xF0;
    context->memory[46] = 0x90;
    context->memory[47] = 0xF0; //"9"
    context->memory[48] = 0x10;
    context->memory[49] = 0xF0;

    context->memory[50] = 0xF0;
    context->memory[51] = 0x90;
    context->memory[52] = 0xF0; //"A"
    context->memory[53] = 0x90;
    context->memory[54] = 0x90;

    context->memory[55] = 0xE0;
    context->memory[56] = 0x90;
    context->memory[57] = 0xE0; //"B"
    context->memory[58] = 0x90;
    context->memory[59] = 0xE0;

    context->memory[60] = 0xF0;
    context->memory[61] = 0x80;
    context->memory[62] = 0x80; //"C"
    context->memory[63] = 0x80;
    context->memory[64] = 0xF0;

    context->memory[65] = 0xE0;
    context->memory[66] = 0x90;
    context->memory[67] = 0x90; //"D"
    context->memory[68] = 0x90;
    context->memory[69] = 0xE0;

    context->memory[70] = 0xF0;
    context->memory[71] = 0x80;
    context->memory[72] = 0xF0; //"E"
    context->memory[73] = 0x80;
    context->memory[74] = 0xF0;

    context->memory[75] = 0xF0;
    context->memory[76] = 0x80;
    context->memory[77] = 0xF0; //"F"
    context->memory[78] = 0x80;
    context->memory[79] = 0x80;
}

uint8_t load_rom(char* name , chip8* context ){
    FILE* rom = fopen("C:\\Users\\User\\Desktop\\cose\\Progetti\\CHIP-8\\roms\\tetris.ch8","rb"); //TODO LEVA STA MERDA 
    int rom_size = 0;
    if(rom == NULL){
        perror("ROM LOADING FAILED");
        return 0;
    }
    fseek(rom, 0, SEEK_END);
    rom_size = ftell(rom);
    rewind(rom);
    int r = fread(context->memory + 512,1,rom_size,rom);
    return 1;
}

void initkeymap(chip8* context){
    uint8_t basickeymap[] = {KEY_X, KEY_ONE, KEY_TWO, KEY_THREE, KEY_Q, KEY_W, KEY_E, KEY_A, KEY_S, KEY_D , KEY_Z, KEY_C, KEY_FOUR, KEY_R, KEY_F, KEY_V};
    memcpy(context->keymap, basickeymap, 16);
}

void DecrementDelayTimer(chip8* context){
    if(context->delay > 0)
        context->delay--;
}

void DecrementSoundTimer(chip8* context){
    if(context->sound > 0)
        context->delay--;
}

chip8* InitChip8(){
    chip8* newchip8 = (chip8*)malloc(sizeof(chip8)); 
    if(newchip8 == 0){
        perror("CHIP8 INIT FAIlED (MEMORY ALLOCATION FAILED)");
        return NULL;
    }
    memset(newchip8->display,0,64*32); 
    memset(newchip8->keymap,0,16);
    memset(newchip8->Vx,0,16);
    memset(newchip8->memory,0,4096);
    initkeymap(newchip8);
    newchip8->PC = 512;
    newchip8->SP = 0;
    newchip8->sound = 0;
    newchip8->delay = 0;
    loadfonts(newchip8);
    return newchip8;
}

uint16_t Fetch(chip8* context){
    uint8_t a = context->memory[context->PC];
    uint8_t b = context->memory[context->PC+1];
    uint16_t instruction = (a << 8) | b;
    if(context->PC + 2 > 4096){
        perror("PC OVERFLOWED MEMORY");
        return 0;
    }
    printf("\nFETCHING %#04x OP BYTE 1 :%#04x (ADDRESS %#04x) BYTE 2: %#04x (ADDRESS %#04x)\n",instruction,a,context->PC ,b,context->PC+1);
    context->PC+=2;
    return instruction;
    
}

uint8_t DecodeAndExecute(chip8* context,uint16_t instruction){
    uint8_t op; //First Nibble  --> extract  with & 0xF000
    uint8_t x;  //Second Nibble --> extract with & 0x0F00
    uint8_t y; //Third Nibble   --> extract with & 0x00F0
    uint8_t n; // Fourth Nibble --> extract with & 0x000F
    uint8_t nn; //Second Byte (THIRD AND FOURTH NIBBLES) extract with & 0x00FF
    uint16_t nnn; // Second, Third and Fourth Nibbles. extract with & 0x0FFF
   
    op = (instruction & 0xF000) >> 12;
    x = (instruction & 0x0F00) >> 8;
    y = (instruction & 0x00F0) >> 4;        
    n = (instruction & 0x000F);
    nn = (instruction & 0x00FF);
    nnn = (instruction & 0x0FFF);

    switch(op){
        case 0x0:
                if(n == 0x0)
                    OP_00E0(context);
                if(n==0xe)
                    OP_00EE(context);
                break;
        case 0x1: 
                OP_1NNN(context,nnn);
                break;
        case 0x2:
                OP_2NNN(context,nnn);
                break;
        case 0x3:
                OP_3XNN(context,x,nn);
                break;
        case 0x4: 
                OP_4XNN(context,x,nn);
                break;
        case 0x5: 
                OP_5XY0(context,x,y);
                break;
        case 0x6:
                OP_6XNN(context,x,nn);
                break;
        case 0x7: 
                OP_7XNN(context,x,nn);
                break;
        case 0x9:
                OP_9XY0(context,x,y);
                break;
        case 0x8:
                switch(n){
                    case 0x0: 
                            OP_8XY0(context,x,y);
                            break;
                    case 0x1:
                            OP_8XY1(context,x,y,1);
                            break;
                    case 0x2: 
                            OP_8XY2(context,x,y,1);
                            break;
                    case 0x3: 
                            OP_8XY3(context,x,y,1);
                            break;
                    case 0x4: 
                            OP_8XY4(context,x,y);
                            break;
                    case 0x5:
                            OP_8XY5(context,x,y);
                            break;
                    case 0x6: 
                            OP_8XY6(context,x,y,1);
                            break;
                    case 0x7:
                            OP_8XY7(context,x,y);
                            break;
                    case 0xe:
                            OP_8XYE(context,x,y,1);
                            break;
                }
                break;
        case 0xA:
                OP_ANNN(context,nnn);
                break;
        case 0xB:
                OP_BNNN(context,nnn,x,1);
                break;
        case 0xC:
                OP_CXNN(context,nn,x);
                break;
        case 0xD:
                OP_DXYN(context,x,y,n);
                break;
        case 0xE:
                switch(y){
                    case 9: 
                        OP_EX9E(context,x);
                        break;
                    case 0xA:
                        OP_EXA1(context,x);
                        break;
                }
                break;
        case 0xF:
                switch(nn){
                    case 0x07:
                        OP_FX07(context,x);
                        break;
                    case 0x15:
                        OP_FX15(context,x);
                        break;
                    case 0x18:
                        OP_FX18(context,x);
                        break;
                    case 0x1E:
                        OP_FX1E(context,x);
                        break;
                    case 0x0A:
                        OP_FX0A(context,x);
                        break;
                    case 0x29:
                        OP_FX29(context,x);
                        break;
                    case 0x33:
                        OP_FX33(context,x);
                        break;
                    case 0x55:
                        OP_FX55(context,x,1);
                        break;
                    case 0x65:
                        OP_FX65(context,x,1);
                        break; 
                }
                break;
        default: printf("ERROR BAD OP\n"); 
    }

}

