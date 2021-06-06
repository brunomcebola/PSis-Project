#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#include "../KVS/configs.h"

#include "../KVS/KVS-Lib/KVS-lib.h"

void my_function(char* key) {
	printf("\n\n[%s] Chave modificada!\n\n", key);
}

int main() {
	char *title = NULL, *script = NULL, *name1 = NULL, *name2 = NULL, *name3 = NULL, *name4 = NULL;
	char *group_id = NULL, *secret = NULL;
	size_t size = 0;

	print_title("Credentials");

	printf("-- Group id: ");
	getline(&group_id, &size, stdin);
	group_id[strcspn(group_id, "\n")] = '\0';
	size = 0;

	printf("-- Secret: ");
	getline(&secret, &size, stdin);
	secret[strcspn(secret, "\n")] = '\0';
	size = 0;

	establish_connection(group_id, secret);

	put_value("title", "Cars");

	put_value("script",
			  "[Inhaling and exhaling deeply]\n\n[Male] OK... Here we go. Focus.\n\nSpeed. I am speed.\n\n[Cars whizzing past]\n\nOne winner, 42 "
			  "losers.\n\nI eat losers for breakfast.\n\n[Car accelerating]\n\nBreakfast.\n\nWait, maybe I\n\nshould have had breakfast.\n\nA little breck-y "
			  "could be good for me.\n\nNo, no, no, stay focused. Speed.\n\n[Cars whizzing]\n\nI'm faster than fast.\n\nQuicker than quick.\n\nI am "
			  "lightning!\n\n[Pounding on door]\n\n[Male] Hey, Lightning! You ready\n\n?[Sheryl Crow:\n\nReal Gone]\n\nOh, yeah. Lightning's "
			  "ready\n\n[engine rewing]\n\n[Engine revs]\n\n[Crowd cheers]\n\nKa-chow!\n\n[Cars zooming]\n\nCars.\n\n[Cars whooshing]\n\n[Both scream]\n\n[All "
			  "cheer]\n\nGet your antenna balls here!\n\nGo, Lightnin'!\n\n- Whoo!\n\n- You got that right, slick. [whistles]\n\n[Air wrench "
			  "whirring]\n\nUh! [screams]\n\n[Engine revs]\n\n- [Male] Welcome back to the Dinoco 400.\n\n- [Crowd cheers]\n\nI'm Bob Cutlass, here\n\nwith "
			  "my good friend, Darrell Cartrip.\n\nWe're midway through what may\n\nbe an historic day for racing.\n\nBob, my oil pressure's\n\nthrough "
			  "the roof.\n\nIf this gets more exciting, they're\n\ngonna have to tow me outta the booth!\n\nRight, Darrell.\n\n[Bob] Three cars are "
			  "tied\n\nfor the season points lead,\n\nheading into the final race\n\nof the season.\n\nAnd the winner of this race will win\n\nthe season "
			  "title and the Piston Cup.\n\nDoes The King, Strip Weathers,\n\nhave one more victory in him\n\nbefore retirement\n\n?[Darrell] He's been "
			  "Dinoco's golden boy\n\nfor years!\n\nCan he win them one last Piston Cup\n\n?[Bob] And, as always, in the\n\nsecond place spot we find "
			  "Chick Hicks.\n\nHe's been chasing\n\nthat tailfin his entire career.\n\n[Darrell] Chick thought\n\nthis was his year.\n\nHis chance to "
			  "finally emerge\n\nfrom The King's shadow.\n\nBut the last thing he expected was...\n\nLightning McQueen!\n\n[Bob] You know, I don't "
			  "think\n\nanybody expected this.\n\nThe rookie sensation\n\ncame into the season unknown.\n\nBut everyone knows him now.\n\n[Darrell] Will "
			  "he be the first rookie\n\nto win a Piston Cup and land Dinoco\n\n?[Bob] The legend, the runner-up,\n\nand the rookie!\n\nThree cars, one "
			  "champion!\n\n[Breaks screeching]\n\nNo you don't.\n\n- [Chuckling]\n\n- Hey!\n\n- [Tires squealing]\n\n- [Crowd booing]\n\nWhat a "
			  "ride!\n\n[Chuckling]\n\nGo get 'em, McQueen!\n\nGo get 'em!\n\n[Female] I love you, Lightning!\n\n- Dinoco is all mine.\n\n- "
			  "[Screaming]\n\n[Darrell] Trouble, turn three!\n\n- Get through that, McQueen.\n\n- [Bob] Huge crash behind the leaders!\n\n[Crowd "
			  "gasps]\n\n[Screaming]\n\n[Giggling]\n\n- [Grunts]\n\n- [Gasps]\n\n[Both screaming]\n\n[Bob] Wait a second, Darrell.\n\nMcQueen is in the "
			  "wreckage.\n\n[Darrell] There's no way the rookie\n\ncan make it through!\n\nNot in one piece, that is.\n\n[Exhaling]\n\nYeah!\n\nLightning! "
			  "Oh!\n\n[Darrell] Look at that!\n\nMcQueen made it through!\n\n[Bob] A spectacular move\n\nby Lightning McQueen!\n\nYeah! Ka-chow!\n\nMcQueen! "
			  "McQueen! McQueen!\n\nMcQueen! McQueen! McQueen!\n\nYeah, McQueen! Ka-chow! [honking]\n\n[Bob] While everyone\n\nheads into the "
			  "pits,\n\nMcQueen stays out to take the lead!\n\nDon't take me out, coach.\n\nI can still race!\n\n[Air wrench whirring]\n\n[Chuckling] What "
			  "do you think\n\n?A thing of beauty.\n\n- McQueen made it!\n\n- [Chick] What\n\n?He's not pitting!\n\nYou gotta get me out there!\n\nLet's go! "
			  "Get me back out there!\n\nMcQueen's not going into the pits!\n\n[Darrell] The rookie fired his\n\ncrew chief. The third this season!\n\n- "
			  "[Bob] Says he likes working alone.\n\n- Go, go!\n\nLooks like Chick\n\ngot caught up in the pits.\n\nYeah, after a stop like that,\n\nhe's "
			  "got a lot of ground to make up.\n\nGet ready, boys,\n\nwe're coming to the restart!\n\n[Crowd cheers]\n\nCome on, come on, come on!\n\nWe "
			  "need tires now!\n\nCome on, let's go!\n\n- No, no, no, no! No tires, just gas!\n\n- [Male] What\n\n?You need tires, you idiot!");

	get_value("title", &title);
	get_value("script", &script);

	printf("Title: %s\n\n", title);
	printf("Script:\n %s\\n", script);

	//

	put_value("name1", "John Doe");

	get_value("name1", &name1);

	printf("Name 1: %s\n", name1);

	//

	register_callback("name1", my_function);

	//

	put_value("name1", "Good morning,\nI'm John Doe!");

	get_value("name1", &name1);

	printf("Name 1: %s\n", name1);

	//

	delete_value("name1");

	free(name1);

	//

	put_value("name2", "Bruce Lee");

	get_value("name2", &name2);

	printf("Name 2: %s\n", name2);

	//

	put_value("name3", "Logan Paul");

	get_value("name3", &name3);

	printf("Name 3: %s\n", name3);

	//

	put_value("name4", "Floyd Maywheather");

	get_value("name4", &name4);

	printf("Name 4: %s\n", name4);

	//

	delete_value("name2");
	delete_value("name3");
	delete_value("name4");

	free(name2);
	free(name3);
	free(name4);

	//

	get_value("name2", &name2);
	get_value("name3", &name3);
	get_value("name4", &name4);
	printf("Nome2 -> %s\n", name2);
	printf("Nome3 -> %s\n", name3);
	printf("Nome4 -> %s\n", name4);

	//

	close_connection();

	return 0;
}