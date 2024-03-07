#include "chip8.h"

int rec_width = 20;
int rec_height = 20;

void RenderDisplay(int height, int width,chip8* context){
   BeginDrawing();
   ClearBackground(RAYWHITE);
   SetTraceLogLevel(LOG_FATAL); 
   for(int i =0; i<height;i++){
                for(int j =0; j<width;j++){
                    if(context->display[i][j] == 1){
                        DrawRectangle(j*rec_width,i*rec_width,rec_width,rec_height,MAROON);
                    }
                    else 
                        DrawRectangle(j*rec_width,i*rec_width,rec_width,rec_height,RAYWHITE);
                }
    }
    EndDrawing();
}



int main(int argc, char** argv){
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1280;
    const int screenHeight = 640;
    
    if(argc == 1){
        printf("Usage:\nchip8-interpreter <rom_path>\n");
        return 1;        
    }

    chip8* context = InitChip8();
    
    if(load_rom(argv[1],context) == 0){
        return 1;
    }

    InitWindow(screenWidth, screenHeight, "CHIP-8");
    //--------------------------------------------------------------------------------------


    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        uint16_t opcode = Fetch(context);

        if(opcode == 0){
            perror("ERROR FETCHING INSTRUCTION");
            return 0;
        }

        DecodeAndExecute(context,opcode);
        DecrementDelayTimer(context);
        DecrementSoundTimer(context);
        RenderDisplay(32,64,context);

    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
} 