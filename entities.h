//This header file contains all the definitions for the player and alien structs as well as the rectangle structs and their associated functions

#include "astronaut.h"
#include "collisions.h"

//ALIEN STRUCT AND ASSOCIATED FUNCTIONS
//---------------------------------------------------------------------------------------------------------------------------------------------
typedef struct AlienShip{
    
    int x;
    int y;
    char alienTop[5]; //top row of ASCII art for the alien ship
    char alienBottom[6]; //bottom row of ASCII art for the alien ship
    char alienTopEmpty[5]; //array of " " empty spaces to remove alien characters from where the alien ship was
    char alienBottomEmpty[6]; 

    Rectangle *rect; //rectangle for collisions
    int length; //variables used to correctly set the bounds of the rectangle
    int height;

    float velocityX;
    float velocityY;
    float maxVelX;
    float maxVelY;

    int timeBetweenMoves; //this is the amount of loops between each move of the alien (used to slow down the alien)
    int timeCounter;  //counts down the time between moves

    float distX; //distance travelled in most recent move in x axis
    float distY; //distance travelled in most recent move in y axis

    float thrustForceX; //in Newtons
    float thrustForceY; //in Newtons

    int mass;// in kg 
    //no drag force as we want the alien to be moving at a constant speed
   
    int dirX; //the horizontal direction the ship is moving in -1 for left and 1 for right
    int dirY; //the vertical direction the ship is moving in -1 for up and 1 for down
    int dirChangeTime; //the time till the alien will change its vertical direction (moving up or moving down)

    int hasShot;    //integer variable indicating whether the enemy has shot or not, the alien must then reload before shooting again
    int reloadTime; //number of game loops between each shot
    int reloadTimer; //the counter for this reload time
    int shootRadius; //how close player has to be for alien to shoot
    Rectangle *detectionRect; //rectangle uesd to check if player is in shooting range

    int hasMutated; //indicates if the alien has captured an astronaut and mutated

    int abductionPosition[2]; //the x and y coords of where abducted astronauts are held
    int abductionThreshold;
    int abductionValue; //a random value between 1 and 100 which if over a threshold the alien attempts to abduct astronauts

    int attemptingToAbduct; //int indicating if alien is attempting to abduct aliens
    int hasAstronaut; //int indicating if alien has an astronaut or not
    int abductionAttemptTime; //how long the alien can attempt to abduct an astronaut before giving up
    int abductionAttemptTimer;
    Astronaut *astronaut;

}AlienShip ;

void ChangeDirection(int *currentDirection){

    *currentDirection = *currentDirection *-1;
}

//randomly chooses between 1 and -1
int PickDirection(){

    if(GenerateRandomBetween(0,1)){ //returns either a 1 or a 0
        return 1;
    }else{
        return -1;
    }
}

AlienShip* NewAlien(int x, int y){

    AlienShip *alien = malloc(sizeof(AlienShip));
    char tempTop[] = " /-\\";
    char tempBottom[] = "<...>";
    char tempTopEmpty[] = "    ";
    char tempBottomEmpty[] = "     ";

    strcpy(alien->alienTop, tempTop);
    strcpy(alien->alienBottom, tempBottom);
    strcpy(alien->alienTopEmpty, tempTopEmpty);
    strcpy(alien->alienBottomEmpty, tempBottomEmpty);

    alien->timeBetweenMoves = 2; 
    alien->timeCounter = alien->timeBetweenMoves;

    alien->length = 6;
    alien->height = 2;

    alien->rect = NewRectangle(x,y, alien->length, alien->height);

    alien->x = x;
    alien->y = y;

    alien->velocityX = 0;
    alien->velocityY = 0;
    alien->maxVelX = 2;
    alien->maxVelY = 2;

    alien->distX = 0;
    alien -> distY = 0;

    alien->thrustForceX = 65; //Newtons
    alien->thrustForceY = 65; //Newtons

    alien->mass = 10; //kg (Not heavy at all haha)
                    
    srand(time(0)); //seeds random generator

    alien->dirX = PickDirection(); //randomly choose -1 or 1 (left or right)
    alien->dirY = PickDirection(); //randomly choose -1 or 1 (up or down)

    alien->dirChangeTime = GenerateRandomBetween(10, 100); //choose a random direction change time between 10 and 100 game loops

    alien->hasShot = FALSE;

    alien->reloadTime = 30;
    alien->reloadTimer = alien->reloadTime;

    alien->shootRadius = 8; 
    alien->detectionRect = NewRectangle(x- alien->shootRadius, y- alien->shootRadius, alien->length + 2*alien->shootRadius, alien->height + 2*alien->shootRadius);

    alien->hasMutated = FALSE;

    alien->abductionPosition[0] = x+2;
    alien->abductionPosition[1] = y+2;

    alien->attemptingToAbduct = FALSE;
    alien->hasAstronaut = FALSE;
    alien->abductionValue = 0;
    alien->abductionAttemptTime = 15; 
    alien->abductionAttemptTimer = 0;

    alien->abductionThreshold = 80;
    
}

void ReloadShot(AlienShip *alien){

    if(alien->reloadTimer <= 0){
         
         alien->reloadTimer = alien->reloadTime;
         alien->hasShot = FALSE;

    }else
    {
        alien->reloadTimer -= 1;  
    }
}

void RegularMoveAlien(AlienShip *alien, float time, int screenMaxX, int screenMaxY, int minY, int minX){

    alien->dirChangeTime -=1;
    alien->timeCounter -= 1;

    float thrustX = alien->dirX * alien->thrustForceX; //thrust force is just max thrust force multiplied by the direction of movement as the alien is constantly moving
    float thrustY = alien->dirY * alien->thrustForceY;

    if(alien->dirChangeTime <= 0 || alien->x <= minX || alien->y <= minY) //if change direction timer is up then change vertical direction and reset timer or if above minY
    {
        ChangeDirection(&alien->dirY);       
        alien->dirChangeTime = GenerateRandomBetween(10, 100); 
    }

    
    if(alien->timeCounter <= 0) //alien is only moved if the counter is zero
    {
        alien->timeCounter = alien->timeBetweenMoves; // reset time counter

        alien->velocityX = CalculateVelocity(thrustX, time, alien->velocityX, 0, alien->mass);
        alien->velocityY = CalculateVelocity(thrustY, time, alien->velocityY, 0, alien->mass);
        
        //Check velocity hasn't exceeded max
        alien->velocityX = ClampValue(alien->velocityX, alien->maxVelX, -alien->maxVelX);
        alien->velocityY = ClampValue(alien->velocityY, alien->maxVelY, -alien->maxVelY);

        alien->distX = alien->velocityX * time;
        alien->distY = alien->velocityY * time;

        alien->x += round(alien->distX); //round it to an integer value to be added to current position
        alien->y += round(alien->distY); 
    }

    ResetBounds(alien->rect, alien->x, alien->y, alien->length, alien->height); //move collision rectangle and detection rectangle with alien
    ResetBounds(alien->detectionRect, alien->x- alien->shootRadius, alien->y- alien->shootRadius, alien->length + 2*alien->shootRadius, alien->height + 2*alien->shootRadius);

    if(HasPassedBorder(alien->rect, (minX+1), 1, screenMaxX, screenMaxY - alien->height) == 1){// check if alien has passed level borders

        if((alien->x) > (screenMaxX-(alien->length)) || (alien->x) < (minX+1)){ //if horizontal border crossed change it horizontal move direction
            ChangeDirection(&alien->dirX);
        }

        if((alien->y) > (screenMaxY-(alien->height)) || (alien->y) < (alien->height)){ //if vertical border crossed change it vertical move direction
            ChangeDirection(&alien->dirY);
        }

        alien->x = ClampInteger(alien->x, screenMaxX-(alien->length), (minX+1));
        alien->y = ClampInteger(alien->y, (screenMaxY - alien->height), alien->height);          
    }

}

void UpdateAbductionPos(AlienShip *alien){ //move abduction position with alien
    alien->abductionPosition[0] = alien->x +1;
    alien->abductionPosition[1] = alien->y +2;
}

void AbductionMove(AlienShip *alien, float time, int screenMaxX, int screenMaxY){

    if(alien->y > 1) //if not at top of screen move up
    { 
        alien->timeCounter -= 1;

         if(alien->timeCounter <= 0)  //alien is only moved if the counter is zero
         {
             alien->timeCounter = alien->timeBetweenMoves * 4; // reset time counter to double the regular amount to give player greater chance to stop them

             alien->dirY = -1;
             float thrustY = alien->dirY * alien->thrustForceY;

             alien->velocityY = CalculateVelocity(thrustY, time, alien->velocityY, 0, alien->mass);
             alien->distY = alien->velocityY * time;
             alien->y += round(alien->distY); 

            UpdateAbductionPos(alien);

            alien->astronaut->x = alien->abductionPosition[0];
            alien->astronaut->y = alien->abductionPosition[1];

            ResetBounds(alien->rect, alien->x, alien->y, alien->length, alien->height); //move collision rectangle and detection rectangle with alien
         }
        
    }else
    {
        alien->hasMutated = TRUE;
        alien->astronaut->hasDied = TRUE;
    }
}

void MoveAlien(AlienShip *alien, float time, int screenMaxX, int screenMaxY, int minX){ //Standard move function of alien which calls specific move functions based on situation

    if(alien->hasAstronaut) //if has an astronaut 
    {
        AbductionMove(alien, time, screenMaxX, screenMaxY);
        return;

    }else if(alien->attemptingToAbduct)  //if trying to abduct astronaut go to appropriate move function
    {
        int minY = screenMaxY-(alien->height) - 2;    //sets a min y not far above bottom level so that aliens are constantly moving close to where the astronauts are
        RegularMoveAlien(alien, time, screenMaxX, screenMaxY, minY, minX);

    }else  //otherwise move the alien normally
    {
        RegularMoveAlien(alien, time, screenMaxX, screenMaxY, 0, minX);
    }

    UpdateAbductionPos(alien);

    if(alien->hasShot)   //Reload shot if has recently fired
    {
        ReloadShot(alien);
    }

    if(!alien->hasAstronaut && !alien->attemptingToAbduct) //calculates random value to determine whether the alien should attempt to abduct astronauts
    {

        alien->abductionValue = GenerateRandomBetween(1,100);

        if(alien->abductionValue > alien->abductionThreshold) 
        {
            alien->attemptingToAbduct = 1;
            alien->abductionAttemptTimer = alien->abductionAttemptTime;
        }

    }

    if(alien->attemptingToAbduct && !alien->hasAstronaut) //count down timer while attempting to abduct 
    {
        alien->abductionAttemptTimer -= 1;

        if(alien->abductionAttemptTimer <= 0)
        {
            alien->attemptingToAbduct = 0;
        }
    }

}


void DestroyAlien(AlienShip *alien){ //frees the memory holding the alien data and reorders the array

    if(alien->hasAstronaut && !alien->hasMutated) //if has abducted an alien and hasn't mutated (which kills the astronaut), release it
    {
        alien->astronaut->isAbducted = FALSE;
    }

    free(alien);
}

void ReorderAlienArray(AlienShip *arr[], int index, int maxAliens){

    if(index == maxAliens-1) //if the destroyed alien was at the end of the array do nothing
    { 
        return;
    }

    for(int i = index+1; i < maxAliens; i++){
        arr[i-1] = arr[i]; 
    }
}

int PlayerInRadius(PlayerShip *player, AlienShip *alien, Rectangle *playerRect, Rectangle *alienDetectRect, int *dirX, int *dirY){ //checks if player is within shooting range and if calculates the direction in order to shoot

    if(DoIntersect(playerRect, alienDetectRect)) //if player is in range
    {
        int xDifference = player->x - alien->x;
        int yDifference = player->y - alien->y;

        xDifference = ClampInteger(xDifference, 1, -1); //normalise the values so that they are either 1, 0, -1
        yDifference = ClampInteger(yDifference, 1, -1);

        *dirX = xDifference;
        *dirY = yDifference;

        return 1;
    }

    return 0;

}

//MUTANT ALIEN STRUCT AND ASSOCIATED FUNCTIONS
//---------------------------------------------------------------------------------------------------------------------------------------------
typedef struct MutantAlien{
    
    int x;
    int y;
    char mutantChar[8]; //top row of ASCII art for the mutant alien ship
    char mutantEmpty[8]; //array of " " empty spaces to remove alien characters from where the alien ship was

    Rectangle *rect; //rectangle for collisions
    int length; //variables used to correctly set the bounds of the rectangle
    int height;

    float velocityX;
    float velocityY;
    float maxVelX;
    float maxVelY;

    int timeBetweenMoves; //this is the amount of loops between each move of the alien (used to slow down the alien)
    int timeCounter;  //counts down the time between moves

    float distX; //distance travelled in most recent move in x axis
    float distY; //distance travelled in most recent move in y axis

    int dirX; //the horizontal direction the ship is moving in -1 for left and 1 for right
    int dirY; //the vertical direction the ship is moving in -1 for up and 1 for down

    float thrustForceX; //in Newtons
    float thrustForceY; //in Newtons

    int mass;// in kg 
    //no drag force as we want the alien to be moving at a constant speed

    int hasShot;    //integer variable indicating whether the enemy has shot or not, the alien must then reload before shooting again
    int reloadTime; //number of game loops between each shot
    int reloadTimer; //the counter for this reload time
    int shootRadius; //how close player has to be for alien to shoot
    Rectangle *detectionRect; //rectangle uesd to check if player is in shooting range

}MutantAlien ;

MutantAlien* NewMutant(int x, int y){

    MutantAlien *mutant = malloc(sizeof(MutantAlien));

    char tempChar[] = "/-|!|-\\";
    char tempEmpty[] = "       ";

    strcpy(mutant->mutantChar, tempChar);
    strcpy(mutant->mutantEmpty, tempEmpty);

    mutant->timeBetweenMoves = 2; 
    mutant->timeCounter = mutant->timeBetweenMoves;

    mutant->length = 7;
    mutant->height = 1;

    mutant->rect = NewRectangle(x,y, mutant->length, mutant->height);

    mutant->x = x;
    mutant->y = y;

    mutant->velocityX = 0;
    mutant->velocityY = 0;
    mutant->maxVelX = 2;
    mutant->maxVelY = 2;

    mutant->distX = 0;
    mutant -> distY = 0;

    mutant->dirX = 1;
    mutant->dirY = 1;

    mutant->thrustForceX = 65; //Newtons
    mutant->thrustForceY = 65; //Newtons

    mutant->mass = 10; //kg (Not heavy at all haha)

    mutant->hasShot = FALSE;

    mutant->reloadTime = 20;
    mutant->reloadTimer = mutant->reloadTime;

    mutant->shootRadius = 14; 
    mutant->detectionRect = NewRectangle(x- mutant->shootRadius, y- mutant->shootRadius, mutant->length + 2*mutant->shootRadius, mutant->height + 2*mutant->shootRadius);
    
}

void MutantReloadShot(MutantAlien *mutant){

    if(mutant->reloadTimer <= 0){
         
         mutant->reloadTimer = mutant->reloadTime;
         mutant->hasShot = FALSE;

    }else
    {
        mutant->reloadTimer -= 1;  
    }
}

void MoveMutant(MutantAlien *mutant, PlayerShip *player, float time, int screenMaxX, int screenMaxY, int minX){

    mutant->timeCounter -= 1;

    if(mutant->timeCounter <= 0) //only move when time counter is 0
    {
        mutant->timeCounter = mutant->timeBetweenMoves; //reset timer

        mutant->dirX = player->x - mutant->x;          //calculate desired direction
        mutant->dirY = player->y - mutant->y;  

        mutant->dirX = ClampInteger(mutant->dirX, 1, -1);     
        mutant->dirY = ClampInteger(mutant->dirY, 1, -1);  

        int thrustX = mutant->thrustForceX * mutant->dirX;
        int thrustY = mutant->thrustForceY * mutant->dirY;

        mutant->velocityX = CalculateVelocity(thrustX, time, mutant->velocityX, 0, mutant->mass);
        mutant->velocityY = CalculateVelocity(thrustY, time, mutant->velocityY, 0, mutant->mass);
        
        //Check velocity hasn't exceeded max
        mutant->velocityX = ClampValue(mutant->velocityX, mutant->maxVelX, -mutant->maxVelX);
        mutant->velocityY = ClampValue(mutant->velocityY, mutant->maxVelY, -mutant->maxVelY);

        mutant->distX = mutant->velocityX * time;
        mutant->distY = mutant->velocityY * time;

        mutant->x += round(mutant->distX); //round it to an integer value to be added to current position
        mutant->y += round(mutant->distY); 
    }

    ResetBounds(mutant->rect, mutant->x, mutant->y, mutant->length, mutant->height); //move collision rectangle and detection rectangle with alien
    ResetBounds(mutant->detectionRect, mutant->x- mutant->shootRadius, mutant->y- mutant->shootRadius, mutant->length + 2*mutant->shootRadius, mutant->height + 2*mutant->shootRadius);

    if(HasPassedBorder(mutant->rect, (minX+1), 1, screenMaxX, screenMaxY - mutant->height) == 1){// check if alien has passed level borders

        if((mutant->x) > (screenMaxX-(mutant->length)) || (mutant->x) < 1){ //if horizontal border crossed change it horizontal move direction
            ChangeDirection(&mutant->dirX);
        }

        if((mutant->y) > (screenMaxY-(mutant->height)) || (mutant->y) < (mutant->height)){ //if vertical border crossed change it vertical move direction
            ChangeDirection(&mutant->dirY);
        }

        mutant->x = ClampInteger(mutant->x, screenMaxX-(mutant->length), (minX+1));
        mutant->y = ClampInteger(mutant->y, (screenMaxY - mutant->height), mutant->height);     
    }

    if(mutant->hasShot)
    {
        MutantReloadShot(mutant);  
    }

}

void DestroyMutant(MutantAlien *mutant){
    free(mutant);
}

void ReorderMutantAlienArray(MutantAlien *arr[], int index, int maxAliens){

    if(index == maxAliens-1) //if the destroyed alien was at the end of the array do nothing
    { 
        return;
    }

    for(int i = index+1; i < maxAliens; i++){
        arr[i-1] = arr[i]; 
    }
}

int PlayerInMutantRadius(PlayerShip *player, MutantAlien *mutant, Rectangle *playerRect, Rectangle *mutantDetectRect, int *dirX, int *dirY){ //checks if player is within shooting range and if calculates the direction in order to shoot

    if(DoIntersect(playerRect, mutantDetectRect)) //if player is in range
    {
        int xDifference = player->x - mutant->x;
        int yDifference = player->y - mutant->y;

        xDifference = ClampInteger(xDifference, 1, -1); //normalise the values so that they are either 1, 0, -1
        yDifference = ClampInteger(yDifference, 1, -1);

        *dirX = xDifference;
        *dirY = yDifference;

        return 1;
    }

    return 0;

}