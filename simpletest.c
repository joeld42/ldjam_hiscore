#include <time.h>

#include "ldjam_hiscore.h"

#define HTTP_IMPLEMENTATION
#include "http.h"                                                                                                                                                            

// Get this from the dashboard
#define SCOREBOARD_ID (47)
#define SCOREBOARD_API_KEY "MY_API_KEY" // Not yet used

typedef struct MyGameInfoStruct {
    int done;
} MyGameInfo;

void errorCallback(LDJam_Context *ctx, int status_code, const char *error_msg)
{
    MyGameInfo *game = (MyGameInfo*)ctx->userdata;   

    printf("SIMPLETEST (ERROR): (status code %d) '%s'\n", status_code, error_msg );

    game->done = 1;
}


void fetchScoreboardCallback( LDJam_Context *ctx, LDJam_Scoreboard *board )
{
    MyGameInfo *game = (MyGameInfo*)ctx->userdata;
    printf("SIMPLETEST: Fetched scoreboard '%s (id %d)'!\n", board->name, board->id );

    printf(" ----- %s ----- \n", board->name );
    for (int i=0; i < board->num_scores; i++ ) {
        printf ( "%2d) %-20s ... %d\n", i+1, 
            board->scores[i].username, 
            board->scores[i].score );
    }

    game->done = 1;
}

void submitScoreCallback( LDJam_Context *ctx, LDJam_Scoreboard *board )
{
    MyGameInfo *game = (MyGameInfo*)ctx->userdata;
    printf("SIMPLETEST: submitted score to scoreboard!\n" );

    // Now fetch the updated scores
    ldjam_fetch_scoreboard( ctx, board, fetchScoreboardCallback, errorCallback );
}

void createScoreboardCallback( LDJam_Context *ctx, LDJam_Scoreboard *board )
{
    MyGameInfo *game = (MyGameInfo*)ctx->userdata;

    printf("SIMPLETEST: Created scoreboard succeeded '%s' (id %d).\n", board->name, board->id );

    // Now submit a score to it!
    int score = 100 + (rand() %100);
    ldjam_submit_highscore( ctx, board, "someplayer", score, submitScoreCallback, errorCallback );
}


int main( int argc, char **argv )
{
    LDJam_Context ctx;
    MyGameInfo game = {0};    

    srand(time(NULL));    

    // TestStuff();
    // exit(1);

    ldjam_init_context( &ctx, SCOREBOARD_API_KEY, &game );

    // Make a new scoreboard
    //ldjam_create_scoreboard( &ctx, "com.ldjam.veryfungame", createScoreboardCallback, errorCallback );

    // Get an existing scoreboard
    LDJam_Scoreboard *board = ldjam_init_scoreboard( &ctx, "tk-sample-board", SCOREBOARD_ID );

    // Submit a new score
    int score = 100 + (rand() %100);
    ldjam_submit_highscore( &ctx, board, "namename", score, submitScoreCallback, errorCallback );

    // Process requests until we're done
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