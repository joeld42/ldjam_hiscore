#include <stdio.h>

#include "raylib.h"

#include "ldjam_hiscore.h"

#define HTTP_IMPLEMENTATION
#include "http.h"                                                                                                                                                            

// TILE DATA
#include "tiledata.c"

#define ARRAY_SZ(A) (sizeof(A) / sizeof(A[0]))

int main()
{
    int screenWidth = 640;
    int screenHeight = 480;

    InitWindow(screenWidth, screenHeight, "LDJam Highscore Sample");

    //Vector2 ballPosition = { (float)screenWidth/2, (float)screenHeight/2 };

    SetTargetFPS(60);       // Set target frames-per-second

    // Make level image
    int mapwidth = 160;
    int mapheight = 120;

    int sz = mapwidth*mapheight*3;
    uint8_t *levelImageData = (uint8_t*)malloc(sz);
    memset( levelImageData, 255, sz);

    for (int ndx=0; ndx < ARRAY_SZ(TILE_DATA) / 64; ndx++) {
        for (int jj=0; jj < 8; jj++) {
            for (int ii=0; ii < 8; ii++) {                
                int pp = TILE_DATA[(ndx * 64) + (jj*8) + ii];
                uint8_t r = TILE_PALETTE[pp*3 + 0];
                uint8_t g = TILE_PALETTE[pp*3 + 1];
                uint8_t b = TILE_PALETTE[pp*3 + 2];

                int mj = ndx/(mapwidth/8);
                int mi = ndx%(mapwidth/8);
                int destndx = ( ((mj*8)+jj)*mapwidth + ((mi*8)+ii) ) * 3;
                levelImageData[destndx+0] = r;
                levelImageData[destndx+1] = g;
                levelImageData[destndx+2] = b;                
            }
        }
    }

    Image levelImage = LoadImagePro( levelImageData, mapwidth, mapheight, UNCOMPRESSED_R8G8B8 );
    Texture2D mapTex = LoadTextureFromImage( levelImage );

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // if (IsKeyDown(KEY_RIGHT)) ballPosition.x += 0.8f;
        // if (IsKeyDown(KEY_LEFT)) ballPosition.x -= 0.8f;
        // if (IsKeyDown(KEY_UP)) ballPosition.y -= 0.8f;
        // if (IsKeyDown(KEY_DOWN)) ballPosition.y += 0.8f;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground((Color)RAYWHITE);

            //DrawTexture( mapTex, 0,0, (Color)WHITE );
            Rectangle sourceRec = { 0, 0, mapwidth, mapheight };
            Rectangle destRec = { 0, 0, screenWidth, screenHeight };
            DrawTexturePro( mapTex, sourceRec, destRec, (Vector2){0,0}, 0.0, (Color)WHITE );

            DrawText("move the ball with arrow keys", 10, 10, 20, (Color)DARKGRAY);

            //DrawCircleV(ballPosition, 50, (Color)MAROON);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}