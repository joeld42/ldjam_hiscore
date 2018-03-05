
#include <stdint.h>

#include "http.h"

#define MAX_REQUESTS (10)

#define MAX_SCOREBOARDS (32)

enum {
	LDJam_Request_Create,
	LDJam_Request_Submit,
};

struct LDJam_Context_Struct;
struct LDJam_Scoreboard_Struct;
typedef struct LDJam_Context_Struct LDJam_Context;
typedef struct LDJam_Scoreboard_Struct LDJam_Scoreboard;

typedef void (*LDJam_ErrorCallback)(LDJam_Context *ctx, int status_code, const char *error_msg);
typedef void (*LDJam_CreateScoreBoard_Callback)( LDJam_Context *ctx, LDJam_Scoreboard *board );
typedef void (*LDJam_SubmitScore_Callback)( LDJam_Context *ctx );

typedef struct LDJam_Request_Struct {
	uint32_t requestType;
	http_t *req;
	void *userdata;

	LDJam_ErrorCallback cb_error;	

	// Scoreboard info
	LDJam_Scoreboard *scoreboard;
	LDJam_CreateScoreBoard_Callback cb_create_success;

	// Submit info
	LDJam_SubmitScore_Callback cb_submit_success;

} LDJam_Request;


typedef struct LDJam_Scoreboard_Struct {
	uint32_t id; // Internal ID
	char name[256];

	// TODO: entries

} LDJam_Scoreboard;


typedef struct LDJam_Context_Struct {
	
	char api_key[48];
	int num_active_requests;
	LDJam_Request active_requests[MAX_REQUESTS];

	int num_scoreboards;
	LDJam_Scoreboard *scoreboards[MAX_SCOREBOARDS];

	// Here is a place to put your user data so you can get to it in callbacks
	void *userdata;

} LDJam_Context;

void ldjam_init_context( LDJam_Context *ctx, const char *api_key, void *userdata );

void ldjam_create_scoreboard( LDJam_Context *ctx, const char *scoreboard_name, 
							  LDJam_CreateScoreBoard_Callback success, 
							  LDJam_ErrorCallback error );

void ldjam_submit_highscore( LDJam_Context *ctx, LDJam_Scoreboard *board, const char *playername, int score,
						     LDJam_SubmitScore_Callback success,
						     LDJam_ErrorCallback error );

void ldjam_update();

