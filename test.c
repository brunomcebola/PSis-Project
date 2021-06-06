#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#include "./KVS/configs.h"

#include "./KVS/KVS-Lib/KVS-lib.h"

void my_function(char* key) {
	printf("\n\n[%s] Chave modificada!\n\n", key);
}

int main() {
	char *secret = "C4E2A5V5B5G2Z3Y5R8O1C8Z3J2K3X1I4", *group_id = "hello";
	char* nome1 = NULL;

	establish_connection(group_id, secret);

	put_value(
		"nome1",
		"[Inhaling and exhaling deeply]\n[Male] OK... Here we go. Focus.\nSpeed. I am speed.\n[Cars whizzing past]\nOne winner, 42 "
		"losers.\nI eat losers for breakfast.\n[Car accelerating]\nBreakfast.\nWait, maybe I\nshould have had breakfast.\nA little breck-y "
		"could be good for me.\nNo, no, no, stay focused. Speed.\n[Cars whizzing]\nI'm faster than fast.\nQuicker than quick.\nI am "
		"lightning!\n[Pounding on door]\n[Male] Hey, Lightning! You ready\n?[Sheryl Crow:\nReal Gone]\nOh, yeah. Lightning's "
		"ready\n[engine rewing]\n[Engine revs]\n[Crowd cheers]\nKa-chow!\n[Cars zooming]\nCars.\n[Cars whooshing]\n[Both scream]\n[All "
		"cheer]\nGet your antenna balls here!\nGo, Lightnin'!\n- Whoo!\n- You got that right, slick. [whistles]\n[Air wrench "
		"whirring]\nUh! [screams]\n[Engine revs]\n- [Male] Welcome back to the Dinoco 400.\n- [Crowd cheers]\nI'm Bob Cutlass, here\nwith "
		"my good friend, Darrell Cartrip.\nWe're midway through what may\nbe an historic day for racing.\nBob, my oil pressure's\nthrough "
		"the roof.\nIf this gets more exciting, they're\ngonna have to tow me outta the booth!\nRight, Darrell.\n[Bob] Three cars are "
		"tied\nfor the season points lead,\nheading into the final race\nof the season.\nAnd the winner of this race will win\nthe season "
		"title and the Piston Cup.\nDoes The King, Strip Weathers,\nhave one more victory in him\nbefore retirement\n?[Darrell] He's been "
		"Dinoco's golden boy\nfor years!\nCan he win them one last Piston Cup\n?[Bob] And, as always, in the\nsecond place spot we find "
		"Chick Hicks.\nHe's been chasing\nthat tailfin his entire career.\n[Darrell] Chick thought\nthis was his year.\nHis chance to "
		"finally emerge\nfrom The King's shadow.\nBut the last thing he expected was...\nLightning McQueen!\n[Bob] You know, I don't "
		"think\nanybody expected this.\nThe rookie sensation\ncame into the season unknown.\nBut everyone knows him now.\n[Darrell] Will "
		"he be the first rookie\nto win a Piston Cup and land Dinoco\n?[Bob] The legend, the runner-up,\nand the rookie!\nThree cars, one "
		"champion!\n[Breaks screeching]\nNo you don't.\n- [Chuckling]\n- Hey!\n- [Tires squealing]\n- [Crowd booing]\nWhat a "
		"ride!\n[Chuckling]\nGo get 'em, McQueen!\nGo get 'em!\n[Female] I love you, Lightning!\n- Dinoco is all mine.\n- "
		"[Screaming]\n[Darrell] Trouble, turn three!\n- Get through that, McQueen.\n- [Bob] Huge crash behind the leaders!\n[Crowd "
		"gasps]\n[Screaming]\n[Giggling]\n- [Grunts]\n- [Gasps]\n[Both screaming]\n[Bob] Wait a second, Darrell.\nMcQueen is in the "
		"wreckage.\n[Darrell] There's no way the rookie\ncan make it through!\nNot in one piece, that is.\n[Exhaling]\nYeah!\nLightning! "
		"Oh!\n[Darrell] Look at that!\nMcQueen made it through!\n[Bob] A spectacular move\nby Lightning McQueen!\nYeah! Ka-chow!\nMcQueen! "
		"McQueen! McQueen!\nMcQueen! McQueen! McQueen!\nYeah, McQueen! Ka-chow! [honking]\n[Bob] While everyone\nheads into the "
		"pits,\nMcQueen stays out to take the lead!\nDon't take me out, coach.\nI can still race!\n[Air wrench whirring]\n[Chuckling] What "
		"do you think\n?A thing of beauty.\n- McQueen made it!\n- [Chick] What\n?He's not pitting!\nYou gotta get me out there!\nLet's go! "
		"Get me back out there!\nMcQueen's not going into the pits!\n[Darrell] The rookie fired his\ncrew chief. The third this season!\n- "
		"[Bob] Says he likes working alone.\n- Go, go!\nLooks like Chick\ngot caught up in the pits.\nYeah, after a stop like that,\nhe's "
		"got a lot of ground to make up.\nGet ready, boys,\nwe're coming to the restart!\n[Crowd cheers]\nCome on, come on, come on!\nWe "
		"need tires now!\nCome on, let's go!\n- No, no, no, no! No tires, just gas!\n- [Male] What\n?You need tires, you idiot!");

	get_value("nome1", &nome1);

	printf("Nome1: %s\n", nome1);

	register_callback("nome1", my_function);

	//

	put_value("nome1", "André Silva");

	get_value("nome1", &nome1);

	printf("Nome1: %s\n", nome1);

	//

	delete_value("nome1");

	//

	put_value("nome1", "Bruno Cebola");

	get_value("nome1", &nome1);

	printf("Nome1: %s\n", nome1);

	//

	close_connection();

	return 0;
}