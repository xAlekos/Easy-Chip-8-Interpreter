#include "chip8.h"

int rec_width = 20;
int rec_height = 20;

uint8_t RenderDisplay(int height, int width,int display[height][width]){
   BeginDrawing();
   ClearBackground(RAYWHITE);
   SetTraceLogLevel(LOG_FATAL); 
   for(int i =0; i<height;i++){
                for(int j =0; j<width;j++){
                    if(display[i][j] == 1){
                        DrawRectangle(j*rec_width,i*rec_width,rec_width,rec_height,MAROON);
                    }
                    else 
                        DrawRectangle(j*rec_width,i*rec_width,rec_width,rec_height,RAYWHITE);
                }
    }
    EndDrawing();
}



int main(){
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1280;
    const int screenHeight = 640;

    InitWindow(screenWidth, screenHeight, "CHIP-8");
    SetTargetFPS(500);
    chip8* context = InitChip8();
    if(load_rom("a",context) == 0){
        perror("ROM LOADING FAILED");
        return 0;
    }
    //--------------------------------------------------------------------------------------


    while (!WindowShouldClose())    // Detect window close button or ESC key
    {

       //if(IsKeyDown(KEY_SPACE)){
        uint16_t opcode = Fetch(context);
        if(opcode == 0){
            perror("ERROR FETCHING INSTRUCTION");
            return 0;
        }
        DecodeAndExecute(context,opcode);
        DecrementDelayTimer(context);
        DecrementSoundTimer(context);
      // }
        BeginDrawing();
        ClearBackground(RAYWHITE);
        for(int i =0; i<32;i++){
            for(int j =0; j<64;j++){
                if(context->display[i][j] == 1){
                    DrawRectangle(j*rec_width,i*rec_width,rec_width,rec_height,MAROON);
                    //DrawRectangleLines(j*rec_width,i*rec_width,rec_width,rec_height,GRAY);
                }
                else 
                    DrawRectangle(j*rec_width,i*rec_width,rec_width,rec_height,RAYWHITE);
                    //DrawRectangleLines(j*rec_width,i*rec_width,rec_width,rec_height,GRAY);
                }
    }
    EndDrawing();
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
} 