#include <time.h>

#include "ldjam_hiscore.h"

#define HTTP_IMPLEMENTATION
#include "http.h"                                                                                                                                                            

typedef struct MyGameInfoStruct {
    int done;
} MyGameInfo;

void errorCallback(LDJam_Context *ctx, int status_code, const char *error_msg)
{
    MyGameInfo *game = (MyGameInfo*)ctx->userdata;   

    printf("ERROR: (status code %d) '%s'\n", status_code, error_msg );

    game->done = 1;
}

void createScoreboardCallback( LDJam_Context *ctx, LDJam_Scoreboard *board )
{
    MyGameInfo *game = (MyGameInfo*)ctx->userdata;

    printf("Created scoreboard succeeded '%s' (id %d).\n", board->name, board->id );

    game->done = 1;
}

int main( int argc, char **argv )
{
    LDJam_Context ctx;
    MyGameInfo game = {0};    

    ldjam_init_context( &ctx, "MY_API_KEY", &game );


    // Make a new scoreboard
    ldjam_create_scoreboard( &ctx, "com.ldjam.veryfungame", createScoreboardCallback, errorCallback );

    while (( ctx.num_active_requests ) || (!game.done))  {
        ldjam_update(&ctx);
        usleep(1000 * 16);
    }
}

#if 0
int main( int argc, char** argv )                                                                                                                          
{                                                                                                                                                       

    http_t* request = http_get( "http://wujood.ddns.net:8080/api/highscore/123", NULL );
    if( !request )
    {
        printf( "Invalid request.\n" );
        return 1;
    }

    http_status_t status = HTTP_STATUS_PENDING;
    int prev_size = -1;
    while( status == HTTP_STATUS_PENDING )
    {
        status = http_process( request );
        if( prev_size != (int) request->response_size )
        {
            printf( "%d byte(s) received.\n", (int) request->response_size );
            prev_size = (int) request->response_size;
        }
    }

    if( status == HTTP_STATUS_FAILED )
    {
        printf( "HTTP request failed (%d): %s.\n", request->status_code, request->reason_phrase );
        http_release( request );
        return 1;
    }

    printf( "\nContent type: %s\n\n%s\n", request->content_type, (char const*)request->response_data );        
    http_release( request );
    return 0;
}
#endif