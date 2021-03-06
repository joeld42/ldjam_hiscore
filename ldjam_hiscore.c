
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include "http.h"
#include "string.h"

#include "ldjam_hiscore.h"

#define HISCORE_SERVER_BASE "http://wujood.ddns.net:8080/api/"

#define LOCAL static

// string type that can be used as a range into a buffer
typedef struct _LDJam_String_Struct
{
	const char *base;
	const char *end;
} _LDJam_String;

LOCAL _LDJam_String _LDJam_NULL_Str = {0};

typedef struct _LDJam_KeyValuePair_Struct
{
	_LDJam_String key;
	_LDJam_String value;
} _LDJam_KeyValuePair;

LOCAL int _ldjam_str_compare_cstr( _LDJam_String strA, const char *cstr )
{
	int expectedLen = (strA.end - strA.base);
	if (expectedLen != strlen(cstr)) {
		return 1;
	}
	return strncmp( strA.base, cstr, expectedLen );

	return 0;
}

LOCAL int _ldjam_str_compare( _LDJam_String strA, _LDJam_String strB )
{
	if ((strA.end - strA.base) != (strB.end - strB.base)) {
		return 1;
	}

	const char *chB = strB.base;
	for (const char *ch = strA.base; ch != strA.end; ch++) {
		if (*ch != *chB++) return 1;
	}

	return 0;
}

LOCAL void _ldjam_str_extract( char *dest_buff, _LDJam_String str )
{
	char *dest = dest_buff;
	for (const char *ch = str.base; ch != str.end; ch++) {
		*dest++ = *ch;
	}
	*dest = '\0';
}

LOCAL void _ldjam_context_push_request( LDJam_Context *ctx, LDJam_Request request )
{	
    if( !request.req )
    {
        printf( "Invalid request.\n" );        
    } else {
    	ctx->active_requests[ctx->num_active_requests++] = request;
    }
}

LOCAL int _ldjam_shouldstrip( char ch )
{
	return (ch==' ') || (ch=='\t') || (ch=='\n') || (ch=='\"');
}

LOCAL void _ldjam_strip_and_unquote( _LDJam_String *str )
{
	while ((*str->base) && (_ldjam_shouldstrip( *str->base )) ) {
		str->base++;
	}

	while ( (str->end > str->base) && (_ldjam_shouldstrip( *(str->end-1) )) ) {
		str->end--;
	}

}

LOCAL int _ldjam_parse_next_pair( const char *ch, _LDJam_KeyValuePair *pair )
{
	// find the next key
	pair->key.base = strchr( ch, '\"' );
	if (!pair->key.base) {
		pair->key = _LDJam_NULL_Str;
		pair->value = _LDJam_NULL_Str;
		return 0;
	}

	pair->key.base++;
	pair->key.end = strchr( pair->key.base, '\"' );
	if (!pair->key.end) {
		pair->key = _LDJam_NULL_Str;
		pair->value = _LDJam_NULL_Str;
		return 0;
	}

	pair->value.base = strchr( pair->key.end+1, ':' );
	if (!pair->value.base) {
		pair->value = _LDJam_NULL_Str;
		return 0;	
	}
	pair->value.base++;

	// Look for ',' or '}', or '[' to start an array
	int isArray = 0;
	pair->value.end = pair->value.base;
	while (pair->value.end) {
		// printf("VAL: '%c'\n", *pair->value.end );
		if ((*pair->value.end=='[')) {
			printf ("Is Array!\n");
			isArray = 1;
		}

		pair->value.end++;

		if (isArray) {
			// If this is an array, consume everything up to the end
			if (*pair->value.end==']') {
				pair->value.end++;
				break;				
			}
		} else if ((*pair->value.end==',') || (*pair->value.end=='}')) {			
			break;			
		}
	}
	if (!pair->value.end) {
		pair->value = _LDJam_NULL_Str;
		return 0;	
	}

	_ldjam_strip_and_unquote( &pair->value );

	return 1;
}

LOCAL void _ldjam_parse_scoreboard_response( LDJam_Context *ctx, const char *json, LDJam_Scoreboard *board )
{	
	printf("_ldjam_parse_scoreboard_response...\n" );
	// For now this is really dumb and doesn't support nested json
	_LDJam_KeyValuePair pair = {0};

	const char *ch = json;
	while (_ldjam_parse_next_pair( ch, &pair )) {	

		char keybuff[256];
		char valbuff[256];
		_ldjam_str_extract( keybuff, pair.key );	
		_ldjam_str_extract( valbuff, pair.value );	
		printf("JSON PAIR: key '%s' : value '%s'\n", keybuff, valbuff );

		// Look for fields we care about
		if (!_ldjam_str_compare_cstr( pair.key, "id")) {
			board->id = strtol( pair.value.base, NULL, 10 );
		}

		ch = pair.value.end + 1;
	}
}

// Find the end of a block by counting braces
LOCAL _LDJam_String _ldjam_parse_block( const char *start )
{
	printf("  _ldjam_parse_block...\n");

	int braceCount = 0;	
	_LDJam_String range;
	range.base = start;
	range.end = start;
	assert( *range.base=='{');
	do {
		if (*range.end=='{') {
			braceCount++;
		} else if (*range.end=='}') {
			braceCount--;		
		}
		range.end++;

	} while ((*range.end) && (braceCount>0));

	return range;
}

LOCAL void _ldjam_parse_score( LDJam_Context *ctx, LDJam_Scoreboard *board, _LDJam_String scoreStr )
{
	LDJam_Score *score = board->scores + board->num_scores;

	const char *ch = scoreStr.base;
	_LDJam_KeyValuePair pair = {0};
	while (_ldjam_parse_next_pair( ch, &pair ) && (ch < scoreStr.end)) {	

		char keybuff[512];
		char valbuff[512];
		_ldjam_str_extract( keybuff, pair.key );	
		

		// Look for fields we care about
		if (!_ldjam_str_compare_cstr( pair.key, "username")) {
			_ldjam_str_extract( score->username, pair.value );	
		} else if (!_ldjam_str_compare_cstr( pair.key, "score")) {
			score->score = strtol( pair.value.base, NULL, 10 );
		}

		ch = pair.value.end + 1;		
	}	

	printf("Got Score: player '%s' score %d\n", score->username, score->score );
	board->num_scores++;

}

LOCAL void _ldjam_parse_entries( LDJam_Context *ctx, LDJam_Scoreboard *board, _LDJam_String arrayStr )
{
	const char *ch = arrayStr.base + 1;
	
	// Reset scoreboard
	board->num_scores = 0;

	while (*arrayStr.base!='{') {
		arrayStr.base++;
	}

	while (arrayStr.base < arrayStr.end) 
	{
		_LDJam_String rangeStr = _ldjam_parse_block( arrayStr.base );

		//char entbuff[512];
		//_ldjam_str_extract( entbuff, rangeStr );
		//printf("ENT BLOCK:\n%s\n", entbuff );

		_ldjam_parse_score( ctx, board, rangeStr );

		arrayStr.base = rangeStr.end;
		do {
			arrayStr.base++;
		} while ((*arrayStr.base==' ') || (*arrayStr.base=='\t') || (*arrayStr.base==',') || (*arrayStr.base=='\n'));

	}


}

LOCAL void _ldjam_parse_fetchscores_response( LDJam_Context *ctx, const char *json, LDJam_Scoreboard *board )
{
	printf("_ldjam_parse_fetchscores_response...\n");
	printf("json is\n---\n%s\n---\n", json );
	
	_LDJam_KeyValuePair pair = {0};

	const char *ch = json;
	while (_ldjam_parse_next_pair( ch, &pair )) {	

		char keybuff[512];
		char valbuff[512];
		_ldjam_str_extract( keybuff, pair.key );	
		_ldjam_str_extract( valbuff, pair.value );	
		//printf("JSON PAIR: key '%s' : value '%s'\n", keybuff, valbuff );

		// Look for fields we care about
		if (!_ldjam_str_compare_cstr( pair.key, "entries")) {
			if (*pair.value.base!='[') {
				printf("WARNING: Expected entries array, got '%s'\n", valbuff );
			} else {
				_ldjam_parse_entries( ctx, board, pair.value );
			}
		}


		ch = pair.value.end + 1;
	}
}

void ldjam_init_context( LDJam_Context *ctx, const char *api_key, void *userdata )
{
	memset( ctx, 0, sizeof(LDJam_Context) );
	strncpy( ctx->api_key, api_key, sizeof(ctx->api_key) );
	ctx->userdata = userdata;
}

void ldjam_create_scoreboard( LDJam_Context *ctx, const char *scoreboard_name, 
							  LDJam_CreateScoreBoard_Callback success, 
							  LDJam_ErrorCallback error )
{

	// TEST
	// {
	// LDJam_Scoreboard *board = (LDJam_Scoreboard*)malloc( sizeof (LDJam_Scoreboard));		
	// memset( board, 0, sizeof(LDJam_Scoreboard) );
	// strcpy( board->name, "Test High score");

	// char *testResponse = "{\"id\":7,\"apikey\":\"Not implemented yet\"}";
	// //char *testResponse2 = "  { \"id\" : 7,\"apikey\" : \"Not implemented yet\" } ";
	// //char *testResponse3 = "{  \"id\" : 7,\n  \"apikey\": \"Not implemented yet\"}";
	// _ldjam_parse_scoreboard_response( ctx, testResponse, board );
	// //_ldjam_parse_scoreboard_response( ctx, testResponse2 );
	// //_ldjam_parse_scoreboard_response( ctx, testResponse3 );

	// printf("Board '%s', id %d\n", board->name, board->id );

	// exit(0);
	// return;
	// }

	// Make a slot for the new scoreboard	
	// FIXME: Use same custom allocator as HTTP or something
	assert( ctx->num_scoreboards < MAX_SCOREBOARDS-1 );
	ctx->scoreboards[ ctx->num_scoreboards ] = (LDJam_Scoreboard*)malloc( sizeof (LDJam_Scoreboard));		
	LDJam_Scoreboard *board = ctx->scoreboards[ ctx->num_scoreboards ];
	ctx->num_scoreboards++;
	memset( board, 0, sizeof(LDJam_Scoreboard) );

	strcpy( board->name, scoreboard_name );


	// FIXME: calculate length, and/or limit scoreboard_name
	char post_data[1024];
	sprintf( post_data, "{\n"
    					 "\"id\": \"0\",\n"
    					 "\"name\": \"%s\",\n"
    					 "\"entries\": []\n"
						 "}", scoreboard_name );

	LDJam_Request request = {0};
	request.requestType = LDJam_Request_Create;	
	request.userdata = ctx->userdata;
	request.cb_error = error;
	request.cb_create_success = success;
	request.scoreboard = board;
	request.req = http_post( HISCORE_SERVER_BASE "highscore/", post_data, strlen(post_data), NULL );

	_ldjam_context_push_request( ctx, request );
}

LDJam_Scoreboard *ldjam_init_scoreboard( LDJam_Context *ctx, 
	const char *scoreboard_name, uint32_t scoreboardId )
{
	// Make a slot for the new scoreboard	
	// FIXME: Use same custom allocator as HTTP or something
	assert( ctx->num_scoreboards < MAX_SCOREBOARDS-1 );
	ctx->scoreboards[ ctx->num_scoreboards ] = (LDJam_Scoreboard*)malloc( sizeof (LDJam_Scoreboard));		
	LDJam_Scoreboard *board = ctx->scoreboards[ ctx->num_scoreboards ];
	ctx->num_scoreboards++;
	memset( board, 0, sizeof(LDJam_Scoreboard) );
	strcpy( board->name, scoreboard_name );
	board->id = scoreboardId;

	return board;
}

void ldjam_submit_highscore( LDJam_Context *ctx, LDJam_Scoreboard *board, const char *playername, int score,
						     LDJam_SubmitScore_Callback success,
						     LDJam_ErrorCallback error )
{
	LDJam_Request request = {0};
	request.requestType = LDJam_Request_Submit;	
	request.userdata = ctx->userdata;
	request.cb_error = error;
	request.cb_submit_success = success;
	request.scoreboard = board;

	char restUrl[256];
	sprintf(restUrl, HISCORE_SERVER_BASE "highscore/%d", board->id );

	printf("Submit score: %s\n", restUrl );

	char post_data[1024];
	sprintf( post_data, "{\n"
    					 "\"id\": \"%d\",\n"
    					 "\"username\": \"%s\",\n"
    					 "\"score\": \"%d\"\n"
						 "}", board->id, playername, score );
// 	{
//     "id": "1",
//     "username": "username",
//     "score": "5"
// }
	printf("SUBMIT SCORE: %s\n\n", post_data );

	request.req = http_post( restUrl, post_data, strlen(post_data), NULL );
	_ldjam_context_push_request( ctx, request );
}

void ldjam_fetch_scoreboard( LDJam_Context *ctx, LDJam_Scoreboard *board,
							 LDJam_FetchScoreboard_Callback success,
						     LDJam_ErrorCallback error )
{
	LDJam_Request request = {0};
	request.requestType = LDJam_Request_FetchScores;	
	request.userdata = ctx->userdata;
	request.cb_error = error;
	request.cb_fetch_success = success;
	request.scoreboard = board;

	char restUrl[256];
	sprintf(restUrl, HISCORE_SERVER_BASE "highscore/%d", board->id );

	request.req = http_get( restUrl, NULL );
	_ldjam_context_push_request( ctx, request );
}

void ldjam_update( LDJam_Context *ctx )
{
	for (int i=0; i < ctx->num_active_requests; i++) {

		if (ctx->active_requests[i].req == NULL) {
			printf("WARN: completed request still in active list.");
			continue;
		}

		http_status_t status = http_process( ctx->active_requests[i].req );
		if (status == HTTP_STATUS_PENDING) {
			// TODO: check for and report progress
		} else {
			// Close this request
			LDJam_Request *request = ctx->active_requests + i;

			if( status == HTTP_STATUS_FAILED ) {

				printf( "HTTP request failed (%d): %s.\n", request->req->status_code, request->req->reason_phrase );
				if (request->cb_error) {
					request->cb_error( ctx, request->req->status_code, request->req->reason_phrase );
				}

			} else {
				printf("request succeeded!\n");				
				printf( "\nContent type: %s\n\n%s\n", request->req->content_type, 
											(char const*)request->req->response_data );        				

				// Now handle the response
				if (request->requestType == LDJam_Request_Create) {

					_ldjam_parse_scoreboard_response( ctx,
									(const char *)request->req->response_data,
									request->scoreboard );
					if (request->cb_create_success) {
						request->cb_create_success( ctx, request->scoreboard );
					}

				} else if (request->requestType == LDJam_Request_Submit) {
					
					// Assume we got "Entry Added!"
					if (request->cb_submit_success) {
						request->cb_submit_success( ctx, request->scoreboard );	
					}

				} else if (request->requestType == LDJam_Request_FetchScores) {

					_ldjam_parse_fetchscores_response( ctx, 
								(const char *)request->req->response_data,
								request->scoreboard );

					if (request->cb_fetch_success) {
						request->cb_fetch_success( ctx, request->scoreboard );
					}

				} else {
					printf("Unexpected request type: %d\n", request->requestType );
				}
			}

			// Release the request			
			http_release( request->req );
			request->req = NULL;
		}
	}		

	// re-compact the list
	for (int i=0; i < ctx->num_active_requests; i++) {
		if (ctx->active_requests[i].req==NULL) {
			ctx->num_active_requests--;
			ctx->active_requests[i] = ctx->active_requests[ctx->num_active_requests];			
		}
	}
	printf("Update: %d active requests\n", ctx->num_active_requests );
}

// TODO: get rid of this
void *readEntireFile( const char *filename, size_t *out_filesz )
{
    FILE *fp = fopen( filename, "r" );
    if (!fp) return NULL;
    
    // Get file size
    fseek( fp, 0L, SEEK_END );
    size_t filesz = ftell(fp);
    fseek( fp, 0L, SEEK_SET );

    void *fileData = malloc( filesz );
    if (fileData)
    {
        size_t result = fread( fileData, filesz, 1, fp );
        
        // result is # of chunks read, we're asking for 1, fread
        // won't return partial reads, so it's all or nothing.
        if (!result)
        {
            free( fileData);
            fileData = NULL;
        }
        else
        {
            // read suceeded, set out filesize
            *out_filesz = filesz;
        }
    }
    
    return fileData;
    
}

void TestStuff() 
{
	size_t sz = 0;
	const char *scoresjson = readEntireFile( "testresponse.json", &sz );
	
	LDJam_Context ctx;
	ldjam_init_context( &ctx, "MY_API_KEY", NULL );
	LDJam_Scoreboard *board = ldjam_init_scoreboard( &ctx, "com.ldjam.veryfungame", 20 );


	_ldjam_parse_fetchscores_response( &ctx, scoresjson, board );
	for (int i=0; i < board->num_scores; i++) {
		printf("%d) %12s  ... %d\n", i+1, board->scores[i].username, board->scores[i].score );
	}

}
