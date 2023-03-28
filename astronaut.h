//Astronaut STRUCT AND ASSOCIATED FUNCTIONS
//---------------------------------------------------------------------------------------------------------------------------------------------

#ifndef ASTRONAUT_H
#define ASTRONAUT_H

#include "collisions.h"

typedef struct Astronaut{

    char personChar[3];
    char personEmpty[3];

    int isFalling; //equals 1 if the astronaut is free falling
    int isAbducted; //equals 1 if astronaut has been picked up by player or alien
    int isSaved; //equals 1 if astronaut was caught by the player 

    int bottomLevel; // bottom of the screen here astronauts stand

    float velY;     //vertical velocity
    float maxVelY;  //maximum vertical velocity

    int x;
    int y;

    int timeBetweenMoves; //this is the amount of loops between each move of the astronaut (used to slow down the astronaut down when falling)
    int timeCounter;  //counts down the time between moves

    Rectangle *rect;

    int heightFallen; //indicates how far astronaut has fallen, it is used to determine if the astronaut fell from high enough to die

    int hasDied; //int indicating if astronaut has dead due to falling from a height

}Astronaut ;

Astronaut* NewPerson(int xPos, int yPos){
    Astronaut *person = malloc(sizeof(Astronaut));

    char tempChar[] = "-i-";
    char tempEmpty[] = "   ";
    strcpy(person->personChar, tempChar);
    strcpy(person->personEmpty, tempEmpty);

    person->isFalling = 0;
    person->isAbducted = 0;
    person->isSaved = 0;

    person->rect = NewRectangle(xPos,yPos, 3, 1);

    person->x = xPos;
    person->y = yPos;

    person->bottomLevel = yPos;

    person->velY = 0;
    person->maxVelY = 5;

    person->timeBetweenMoves = 5;
    person->timeCounter = person->timeBetweenMoves;

    person->hasDied = FALSE;

    return person;

}

void DestroyAstronaut(Astronaut *person){
    free(person);
}

void MovePerson(Astronaut *person, float time){

    if(person->y < person->bottomLevel && !person->isAbducted && !person->isSaved && !person->isFalling) //if above the ground and not being abducted, the astronaut must be falling
    {
        person->isFalling = TRUE;
        person->heightFallen = person->bottomLevel - person->y; //calculate the height that the astronaut is falling from
    }

    if(person->isFalling)
    {
         person->timeCounter -= 1;

         if(person->timeCounter <= 0)  //used to slow down the astronaut down when falling to make it easier to catch by the player
         {
            person->timeCounter = person->timeBetweenMoves;
            person->velY = CalculateVelocity(4, time, person->velY, 0.5, 1);
            person->velY = ClampValue(person->velY, person->maxVelY, -person->maxVelY);

            float distY = person->velY * time;
            person->y += round(distY); //round it to an integer value to be added to current position

            if(person->y == person->bottomLevel) //if a person hits the ground while falling from too high they die otherwise they survive
            {
                if(person->heightFallen > 6) 
                {
                    person->hasDied = TRUE;
                }else
                {
                    person->isFalling = FALSE;
                }
            }
         }  
    
    }

    ResetBounds(person->rect, person->x, person->y, 3, 1);

}

//reorders the astronaut array to ensure astronauts that are alive are stored at the front end of the array with no gaps
void ReorderAstronautArray(Astronaut *arr[], int index, int maxAstronauts){

    if(index == maxAstronauts-1)  //if the killed astronaut was at the end of the array do nothing
    { 
        return;
    }

    for(int i = index+1; i < maxAstronauts; i++){
        arr[i-1] = arr[i]; 
    }
}


#endif