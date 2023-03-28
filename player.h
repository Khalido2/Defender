//PLAYER STRUCT AND ASSOCIATED FUNCTIONS
//---------------------------------------------------------------------------------------------------------------------------------------------

#ifndef PLAYER_H
#define PLAYER_H

#include "collisions.h"
#include "astronaut.h"

typedef struct PlayerShip{
    
    int x;
    int y;
    char playerRight[9]; //ASCII art of ship to the right
    char playerLeft[9]; //ASCII art of ship to the left
    char playerChar[9]; //current ASCII art of ship to be displayed
    char playerEmpty[9]; //array of " " empty spaces to remove player characters from where the player was

    Rectangle *rect; //rectangle for collisions
    int length; //variables used to correctly set the bounds of the rectangle
    int height;

    float velocityX;
    float velocityY;
    float maxVelX;
    float maxVelY;

    float distX; //distance travelled in most recent move in x axis
    float distY; //distance travelled in most recent move in y axis

    float thrustForceX; //in Newtons
    float thrustForceY; //in Newtons

    int mass;// in kg 
    float dragConstant;//the drag coefficient

    int movingRight; //effectively a boolean variable to represent if the player is moving right or left

    int hasDied; //indicates if the player has died

    int hasAstronaut; //is equal to 1 if player is carrying an astronaut
    int astronautHoldPos[2]; //the position of any astronauts being held
    Astronaut* astronaut;

}PlayerShip ;


PlayerShip* NewPlayer(int x, int y){
    PlayerShip *player = malloc(sizeof(PlayerShip));
    char tempRight[] = "}[>]'I=>"; //)._^==]> old design
    char tempLeft[] = "<=I'[<]{";          //<[==^_.(
    char tempEmpty[] = "        ";
    strcpy(player->playerRight, tempRight);
    strcpy(player->playerLeft, tempLeft);
    strcpy(player->playerChar, tempEmpty);
    strcpy(player->playerEmpty, tempEmpty);
    
    player->length = 9; //the length of the player's ASCII sprite    
    player->height = 1; //the height of the player's ASCII sprite
    player->rect = NewRectangle(player->x, player->y, player->length,  player->height);

    player->movingRight = TRUE;
 
    player->x = x;
    player->y = y;
    player->velocityX = 0;
    player->velocityY = 0;
    player->maxVelX = 10;
    player->maxVelY = 2;

    player->distX = 0;
    player -> distY = 0;

    player->thrustForceX = 250; //Newtons
    player->thrustForceY = 80; //Newtons

    player->mass = 10; //kg (Not heavy at all haha)
    player->dragConstant = 0.001; //slightly less than the drag coefficient of Subsonic Transport Aircraft */

    player->hasDied = FALSE;

    player->hasAstronaut = FALSE;
    player->astronautHoldPos[0] = player->x+2;
    player->astronautHoldPos[1] = player->y+1;

    return player;
};

void PlayerMove(PlayerShip *player, float thrustForceX, float thrustForceY, float time, int screenMaxX, int screenMaxY){

       player->velocityX = CalculateVelocity(thrustForceX, time, player->velocityX, player->dragConstant, player->mass);
       player->velocityY = CalculateVelocity(thrustForceY, time, player->velocityY, player->dragConstant, player->mass);
       
       //Check velocity hasn't exceeded max
       player->velocityX = ClampValue(player->velocityX, player->maxVelX, -player->maxVelX);
       player->velocityY = ClampValue(player->velocityY, player->maxVelY, -player->maxVelY);

       player->distX = player->velocityX * time;
       player->distY = player->velocityY * time;
       player->x += round(player->distX); //round it to an integer value to be added to current position
       player->y += round(player->distY);

       ResetBounds(player->rect, player->x, player->y, player->length, player->height);

       player->astronautHoldPos[0] = player->x+2;
       player->astronautHoldPos[1] = player->y+1;

       if(HasPassedBorder(player->rect, 1, 1, screenMaxX, screenMaxY-player->height) == 1){ 
            player->x = ClampInteger(player->x, screenMaxX-(player->length), 1);
            player->y = ClampInteger(player->y, (screenMaxY-(player->height)), player->height);          
       }

        //ensure correct sprite is displayed
       if(player->movingRight){
           strcpy(player->playerChar, player->playerRight);
       }else{
            strcpy(player->playerChar, player->playerLeft);
       }

       if(player->hasAstronaut)
       {
           player->astronaut->x = player->astronautHoldPos[0];
           player->astronaut->y = player->astronautHoldPos[1];
       }
}

void ResetPlayer(int x, int y, PlayerShip *player){//resets player by setting velocity to zero and moving their position back to a respawn position

    player->velocityX = 0;
    player->velocityY = 0;

    player->x = x;
    player->y = y;

}


#endif