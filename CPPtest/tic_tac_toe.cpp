#include "CPPtest.cpp"

//le board
// 
//   
//  1#2#3
//  #####
//  4#5#6
//  #####
//  7#8#9

//0, 0 is bottom left, 5, 5 is top right
int vertLine1[2][2] = { {1, 0}, {1, 3} };
int vertLine2[2][2] = { {3, 0}, {3, 3} };

int horizLine1[2][2] = { {0, 1}, {3, 1} };
int horiziLne2[2][2] = { {0, 3}, {3, 3} };

//0 indicates an open spot, 1 indicates a robot location, 2 indicates a player location
int boardState[9] = { 0 };

int detectWin();
int getPlayerInput();
POSE boardStateToPose(int);

path drawBoardInit() {
	path boardPath;
	return boardPath;
}

path robotMove() {
	path robotPath;

	srand(time(NULL));

	int robotMove = rand() % (9 + 1 - 1) + 1;

	POSE homePose;
	homePose.w.y = 1; //pen up

	boardState[robotMove] = 2;
	POSE drawPose = boardStateToPose(robotMove);

	path toPosition = interpolate(homePose, drawPose, 2); //go to writing position
	path markSpot = createCharPath('O', drawPose);		//write
	path backHome = interpolate(drawPose, homePose, 2);	//go back to 0, 0

	return combinePaths(combinePaths(toPosition, markSpot), backHome);
}

path playerMove() {
	POSE homePose;
	homePose.w.y = 1; //pen up

	int playerMove = getPlayerInput();

	boardState[playerMove] = 2;
	POSE drawPose = boardStateToPose(playerMove);

	path toPosition = interpolate(homePose, drawPose, 2); //go to writing position
	path markSpot = createCharPath('X', drawPose);		//write
	path backHome = interpolate(drawPose, homePose, 2);	//go back to 0, 0

	return combinePaths(combinePaths(toPosition, markSpot), backHome);
}

int getPlayerInput() {
	cout << "enter your move:" << endl;
	int invalid = 1;
	int input;
	while (invalid) {

		invalid = 0;

		cin >> input;

		if (input > 9 || input < 1) {
			invalid = 1;
		}
		else if (boardState[input] != 0){
			invalid = 1;
		}
		if (invalid) {
			cout << "you have entered an invalid space or tried to take a position that has been taken. please enter a valid position." << endl;
		}
	}
	return input;
}

int detectWin() {
	int sum = 0;
	for (int i = 0; i < 8; i++) {
		sum += boardState[i];
	}
	if (sum < 6) {
		cout << "nobody has won yet. the game continues..." << endl;
		return 0;
	}
	return 1;

}

POSE boardStateToPose(int poseNum) {
	POSE boardPose = { 0 };

	switch (poseNum) {
	case 1:
		boardPose.w.x = 1;
		boardPose.w.y = 1;
		boardPose.w.z = 1;
		return boardPose;
		break;

	case 2:
		boardPose.w.x = 2;
		boardPose.w.y = 1;
		boardPose.w.z = 1;
		return boardPose;
		break;

	case 3:
		boardPose.w.x = 3;
		boardPose.w.y = 1;
		boardPose.w.z = 1;
		return boardPose;
		break;

	case 4:
		boardPose.w.x = 2;
		boardPose.w.y = 1;
		boardPose.w.z = 1;
		return boardPose;
		break;

	case 5:
		boardPose.w.x = 2;
		boardPose.w.y = 2;
		boardPose.w.z = 1;
		return boardPose;
		break;

	case 6:
		boardPose.w.x = 2;
		boardPose.w.y = 3;
		boardPose.w.z = 1;
		return boardPose;
		break;

	case 7:
		boardPose.w.x = 3;
		boardPose.w.y = 1;
		boardPose.w.z = 1;
		return boardPose;
		break;

	case 8:
		boardPose.w.x = 3;
		boardPose.w.y = 2;
		boardPose.w.z = 1;
		return boardPose;
		break;

	case 9:
		boardPose.w.x = 3;
		boardPose.w.y = 3;
		boardPose.w.z = 1;
		return boardPose;
		break;
	}
}
