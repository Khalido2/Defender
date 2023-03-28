#include<stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include<time.h> 
#include <math.h>
#include <string.h>
#include "collisions.h"
#include "astronaut.h"
#include "player.h"
#include "entities.h"
#include "bullets.h"
#include "explosions.h"
#include "particles.h"

#define PLAYER_COLOR_PAIR               1     
#define BULLET_COLOR_PAIR               2  
#define ALIEN_COLOR_PAIR                3  
#define ALIEN_BULLET_COLOR_PAIR         4  
#define MUTANT_ALIEN_COLOR_PAIR         5  
#define TERRAIN_COLOR_PAIR              6  

#define MIN_SPAWN_TIME                  60
#define MAX_SPAWN_TIME                  120

#define DELAYTIME                       0.3  
#define EXIT                            'q'
#define MAX_BULLETS                     3  
#define MAX_ALIEN_BULLETS               10 
#define MAX_ALIENS                      3  
#define MAX_MUTANTS                     6  
#define MAX_ASTRONAUTS                  10  
#define MAX_EXPLOSIONS                  15  
#define MAX_PARTICLES                   5 

#define PLAYER_BULLET_LIFETIME          18  
#define ALIEN_BULLET_LIFETIME           20 

#define PLAYER_BULLET_SPEED             3  
#define ALIEN_BULLET_SPEED              1  

#define ALIEN_KILL_SCORE                150

#define PLAYER_SPAWN_X                  5
#define PLAYER_SPAWN_Y                  2
  
PlayerShip* NewPlayer(int, int);
void PlayerMove(PlayerShip*, float, float, float, int, int);
void ResetPlayer(int, int, PlayerShip*);

AlienShip* NewAlien(int, int);
void MoveAlien(AlienShip*, float, int, int, int);
void ReorderAlienArray(AlienShip *arr[], int, int);
int PlayerInRadius(PlayerShip*, AlienShip*, Rectangle*, Rectangle*, int*, int*);

MutantAlien* NewMutant(int, int);
void MoveMutant(MutantAlien*, PlayerShip*, float, int, int, int);
void DestroyMutant(MutantAlien*);
void ReorderMutantAlienArray(MutantAlien *arr[], int, int);
int PlayerInMutantRadius(PlayerShip*, MutantAlien*, Rectangle*, Rectangle*, int*, int*);

Astronaut* NewPerson(int, int);
void MovePerson(Astronaut*, float);
void DestroyAstronaut(Astronaut*);
void ReorderAstronautArray(Astronaut *arr[], int, int);

PlayerBullet* NewBullet(float, int, int, int, int);
void MoveBullet(PlayerBullet*);
void ReorderBulletArray(PlayerBullet *arr[], int, int);

AlienBullet* NewAlienBullet(float, int, int, int, int, int);
void MoveAlienBullet(AlienBullet*);
void DestroyAlienBullet(AlienBullet*);
void ReorderAlienBulletArray(AlienBullet *arr[], int, int);

Explosion* NewExplosion(int, int);
void AnimateExplosion(Explosion*);
void DestroyExplosion(Explosion*);
void ReorderExplosionArray(Explosion *arr[], int, int);

SpawnParticle* NewSpawnParticle(int, int);
void AnimateSpawnParticle(SpawnParticle*);
void ReorderSpawnParticleArray(SpawnParticle *arr[], int, int);

int DoIntersect(Rectangle*, Rectangle*); 
float ClampValue(float, float, float);
int GenerateRandomBetween(int, int);

//Initialise explosions and spawn particles array  ----> these variables were made global because they need to be accessed in loads of functions across the script
Explosion *explosions[MAX_EXPLOSIONS];
int numExplosions = 0;

SpawnParticle *particles[MAX_PARTICLES];
int numParticles = 0;
//---------------------------------------------------------------------------------------------

void DrawBorders(WINDOW *screen) { //draws window borders

    int x, y;

    getmaxyx(screen, y, x); // get screen bounds

    mvwprintw(screen, 0, 0, "+"); //Draw "+" in the corners of the screen
    mvwprintw(screen, y - 1, 0, "+");
    mvwprintw(screen, 0, x - 1, "+");
    mvwprintw(screen, y - 1, x - 1, "+"); 

    for (int i = 1; i < (y - 1); i++) { //Draw lines on the sides of the screen
        mvwprintw(screen, i, 0, "|");
        mvwprintw(screen, i, x - 1, "|");
    } 
    
    for (int j = 1; j < (x - 1); j++) { //Draw lines on the top and bottom of the screen
        mvwprintw(screen, 0, j, "-");
        mvwprintw(screen, y - 1, j, "-");
    }

}

void DrawSomeBorders(WINDOW *screen){

    int x, y;

    getmaxyx(screen, y, x); // get screen bounds

    mvwprintw(screen, 0, 0, "+"); //Draw "+" in the corners of the screen
    mvwprintw(screen, y - 1, 0, " "); 
    mvwprintw(screen, 0, x - 1, "+");
    mvwprintw(screen, y - 1, x - 1, " "); 

    for (int i = 1; i < (y - 1); i++) { //Draw lines on the sides of the screen
        mvwprintw(screen, i, 0, "|");
        mvwprintw(screen, i, x - 1, "|");
    } 

     for (int j = 1; j < (x - 1); j++) { //Draw lines on the top and bottom of the screen
        mvwprintw(screen, 0, j, "-");
        mvwprintw(screen, y - 1, j, " ");
    }


}

void CleanScreen(WINDOW *screen) { //fills screen with blank spaces in order to remove characters of all the entities drawn in the last game loop
    int x, y;

    getmaxyx(screen, y, x); // get screen bounds

    for(int i = 0; i < x; i++){

        for(int j = 0; j < y; j++){

            mvwprintw(screen, j, i, " ");
        }
    }

}

void DrawRectangle(Rectangle *rect, WINDOW *playArea, char *string){ //draw four corners of a rectangle (Used for debugging purposes)

    mvwprintw(playArea, rect->bLeftCorner[1], rect->bLeftCorner[0], "%s", string);
    mvwprintw(playArea, rect->tLeftCorner[1], rect->tLeftCorner[0], "%s", string);
    mvwprintw(playArea, rect->bRightCorner[1], rect->bRightCorner[0], "%s", string);
    mvwprintw(playArea, rect->tRightCorner[1], rect->bRightCorner[0], "%s", string);

}

void DrawDivider(WINDOW *screen) { //Draws the bottom border of the score screen
    int x, y;
    getmaxyx(screen, y, x); // get screen bounds

     for (int i = 1; i < (x - 1); i++) { //Draw lines on the bottom of the screen
        mvwprintw(screen, y - 1, i, "-");
    }

}

void CreateExplosion(int x, int y){

    if(numExplosions < MAX_EXPLOSIONS)
    {
        explosions[numExplosions] = NewExplosion(x,y);
        numExplosions += 1;
    }

}

void CreateSpawnParticle(int x, int y){

    if(numParticles < MAX_PARTICLES)
    {
        particles[numParticles] = NewSpawnParticle(x,y);
        numParticles += 1;
    }

}

void PlayerHit(PlayerShip *player, int *playerLives){

    *playerLives -= 1;     //take a life from player
    player->hasDied = TRUE;
    CreateExplosion(player->x, player->y); 
}

void SetUpAstronauts(Astronaut *astronauts[], WINDOW *screen, int minX, int maxX){  //places astronauts on the screen

    int  xPosition, xmax, bottomLevel, spacing;
    getmaxyx(screen, bottomLevel, xmax);

    spacing = minX;

    for(int i = 0; i < MAX_ASTRONAUTS; i++){

        xPosition = GenerateRandomBetween(spacing, spacing + (maxX/MAX_ASTRONAUTS)); //astronauts given a small area to spawn in on screen to add a little variation
        spacing += (maxX/MAX_ASTRONAUTS);
        astronauts[i] = NewPerson(xPosition, bottomLevel-1);

    }
}


//Handles user input
void HandleUserInput(int userInput, PlayerShip *player, float *thrustForceX, float *thrustForceY, PlayerBullet *bullets[], int *numBullets){

      *thrustForceX = 0;
      *thrustForceY = 0;

       if(userInput == KEY_RIGHT)  //right move
       {   
             *thrustForceX = player->thrustForceX;
             player->movingRight = true;

       }else if(userInput == KEY_LEFT) //left move
       {
             *thrustForceX = -player->thrustForceX;
             player->movingRight = false;

       }else if(userInput == KEY_UP) //up move
       {
            *thrustForceY = -player->thrustForceY;

       }else if(userInput == KEY_DOWN) //down move
       {
            *thrustForceY = player->thrustForceY;

       }else if(userInput == ' ') //shoot
       {

           if(*numBullets < MAX_BULLETS)
           {

               if(player->movingRight)
               {
                   bullets[*numBullets] = NewBullet(PLAYER_BULLET_SPEED, player->x + 1, player->y, PLAYER_BULLET_LIFETIME, 1);
               }else
               {
                    bullets[*numBullets] = NewBullet(PLAYER_BULLET_SPEED, player->x -1, player->y, PLAYER_BULLET_LIFETIME, -1);
               }

              
               *numBullets += 1;
           }
       }
}

void CheckCollisions(PlayerShip *player, int *playerLives, PlayerBullet *bullets[], int *numBullets, AlienBullet *alienBullets[], int *numAlienBullets, AlienShip *aliens[], int *numAliens, WINDOW *playArea, int *score){

    int hasCollided = FALSE;

    //iterate through aliens and check if they collide with any bullets
    for(int i = 0; i < *numAliens; i++){

        hasCollided = FALSE;

        for(int j = 0; j < *numBullets; j++){

            if(DoIntersect(aliens[i]->rect, bullets[j]->rect)) //if they do intersect
            { 
                mvwprintw(playArea, aliens[i]->y, aliens[i]->x, aliens[i]->alienTopEmpty); //draw blank space where they were
                mvwprintw(playArea, aliens[i]->y+1, aliens[i]->x, aliens[i]->alienBottomEmpty); 
                mvwprintw(playArea, bullets[j]->y, bullets[j]->x, bullets[j]->bulletEmpty);

                CreateExplosion(aliens[i]->x, aliens[i]->y); //create an explosion

                DestroyAlien(aliens[i]);
                ReorderAlienArray(aliens, i, MAX_ALIENS);

                DestroyBullet(bullets[j]);
                ReorderBulletArray(bullets, j, MAX_BULLETS);

                *score += ALIEN_KILL_SCORE; //increment score

                *numBullets -= 1;
                *numAliens -= 1;

                i--;

                hasCollided = TRUE; //set to true to indicate that a collision with this alien has already happened

                break; //break out of loop as this alien can only be shot once 
            }
        }

        if(!hasCollided)
        {
            int dirX = 0; //used as direction values for any alien bullets if player in shoot radius
            int dirY = 0;

            if(DoIntersect(player->rect, aliens[i]->rect))
            {
                mvwprintw(playArea, aliens[i]->y, aliens[i]->x, aliens[i]->alienTopEmpty); //draw blank space where they were
                mvwprintw(playArea, aliens[i]->y+1, aliens[i]->x, aliens[i]->alienBottomEmpty);

                CreateExplosion(aliens[i]->x, aliens[i]->y); //create an explosion

                DestroyAlien(aliens[i]);
                ReorderAlienArray(aliens, i, MAX_ALIENS);
                *numAliens -= 1;
                i--;

                PlayerHit(player, playerLives);

                hasCollided = TRUE;

            } else if(!aliens[i]->hasAstronaut && PlayerInRadius(player, aliens[i], player->rect, aliens[i]->detectionRect, &dirX, &dirY)) //else if the player is in the aliens attack radius, shoot!
            {
                if(!aliens[i]->hasShot)
                {
                    if(*numAlienBullets < MAX_ALIEN_BULLETS)
                    {
                        aliens[i]->hasShot = TRUE;
                        alienBullets[*numAlienBullets] = NewAlienBullet(ALIEN_BULLET_SPEED, aliens[i]->x, aliens[i]->y, ALIEN_BULLET_LIFETIME, dirX, dirY);
                        *numAlienBullets += 1;

                    }

                }
            } 
        }
    }

    for(int i = 0; i < *numAlienBullets; i++){ //iterate through all of the enemy bullets and check if any have collided with the player

        if(DoIntersect(player->rect, alienBullets[i]->rect))
        {
            PlayerHit(player, playerLives);

            DestroyAlienBullet(alienBullets[i]);
            ReorderAlienBulletArray(alienBullets, i, MAX_ALIEN_BULLETS);
            *numAlienBullets -= 1;
            i--;
        }
    }
}

void CheckAstronautState(PlayerShip *player, AlienShip *aliens[], int *numAliens, Astronaut *astronauts[], int *numAstronauts,  PlayerBullet *bullets[], int *numBullets, MutantAlien *mutants[], int *numMutants, WINDOW *playArea, int *gameOver){

    for(int i = 0; i < *numAstronauts; i++){  //iterate through all of the astronauts

        for(int j = 0; j < *numAliens; j++){ //iterate through all aliens to see if astronaut has mutated or been abducted

            if(aliens[j]->attemptingToAbduct && !astronauts[i]->isAbducted && !astronauts[i]->hasDied)
            {
                if((aliens[j]->abductionPosition[0] + 1) == astronauts[i]->x && (aliens[j]->abductionPosition[1] + 1) == astronauts[i]->y)
                {
                    aliens[j]->astronaut = astronauts[i];
                    aliens[j]->hasAstronaut = TRUE;
                    aliens[j]->attemptingToAbduct = FALSE;
                    astronauts[i]->isAbducted = TRUE; 
                }
            }

           if(aliens[j]->hasMutated) //if alien has reached the top with an astronaut and has mutated delete it and the astronaut and spawn a mutant alien
           {

               int x = aliens[j]->x;
               DestroyAlien(aliens[j]);
               ReorderAlienArray(aliens, j, MAX_ALIENS);
               *numAliens -= 1;
               j--;
               
               if(*numMutants < MAX_MUTANTS) 
               {
                   mutants[*numMutants] = NewMutant(x, 1); //spawn mutant alien at the top of the screen
                   *numMutants += 1;
               }
           }
        }

        if(astronauts[i]->isFalling && !astronauts[i]->hasDied)
        {
            for(int k = 0; k < *numBullets; k++){ //iterate through all bullets to see if a falling astronaut has been shot by the player

            if(DoIntersect(bullets[k]->rect, astronauts[i]->rect))
            {
                astronauts[i]->hasDied = TRUE;

                mvwprintw(playArea, bullets[k]->y, bullets[k]->x, bullets[k]->bulletEmpty); //draw blank space where they were

                DestroyBullet(bullets[k]);
                ReorderBulletArray(bullets, k, MAX_BULLETS);
                *numBullets -= 1;
                
                break; //break from loop as astronaut can only be shot once
            }
        }
        }


        if(astronauts[i]->isFalling && !astronauts[i]->hasDied && !player->hasAstronaut && DoIntersect(player->rect, astronauts[i]->rect)) //if astronaut is falling and collides with player, pick them up
        {
            astronauts[i]->isFalling = FALSE;
            astronauts[i]->isSaved = TRUE;
                
            player->astronaut = astronauts[i];
            player->hasAstronaut = TRUE;
        }

        if(astronauts[i]->isSaved && !astronauts[i]->hasDied && astronauts[i]->y == astronauts[i]->bottomLevel) //if the astronaut is saved and is on the bottom level in a space not occupied by another astronaut drop off there
        {
            for(int l = 0; l < *numAstronauts; l++){  //iterate through all the other astronauts to check if there is a free space to be dropped off

                if(l != i && DoIntersect(astronauts[l]->rect, astronauts[i]->rect)) //if the astronaut is colliding with another astronaut, return otherwise drop to the ground
                {
                    return;
                }
            }

            astronauts[i]->isSaved = FALSE;
            player->hasAstronaut = FALSE;
        }

        if(astronauts[i]->hasDied) //if astronaut has died due to falling or being mutated/shot, delete it
        {
            mvwprintw(playArea, astronauts[i]->y, astronauts[i]->x, astronauts[i]->personEmpty);
            DestroyAstronaut(astronauts[i]);
            ReorderAstronautArray(astronauts, i, MAX_ASTRONAUTS);
            *numAstronauts -= 1;
            i--;
        
            if(*numAstronauts <= 0) //end game if all of the astronauts have died
            {
                *gameOver = TRUE;
            }
        }

    }
}

//As check collision function is too large I've made the code handling mutant alien collisions separate
void HandleMutantCollisions(PlayerShip *player, int *playerLives, PlayerBullet *bullets[], int *numBullets, AlienBullet *alienBullets[], int *numAlienBullets, MutantAlien *mutants[], int *numMutants, WINDOW *playArea, int *score){

    int hasCollided = FALSE;

    //iterate through mutant aliens and check if they collide with any bullets
    for(int i = 0; i < *numMutants; i++){

        for(int j = 0; j < *numBullets; j++){

            if(DoIntersect(mutants[i]->rect, bullets[j]->rect)) //if they do intersect
            { 
                mvwprintw(playArea, mutants[i]->y, mutants[i]->x, mutants[i]->mutantEmpty); //draw blank space where they were
                mvwprintw(playArea, bullets[j]->y, bullets[j]->x, bullets[j]->bulletEmpty);

                CreateExplosion(mutants[i]->x, mutants[i]->y); //create an explosion

                DestroyMutant(mutants[i]);
                ReorderMutantAlienArray(mutants, i, MAX_MUTANTS);

                DestroyBullet(bullets[j]);
                ReorderBulletArray(bullets, j, MAX_BULLETS);

                *score += ALIEN_KILL_SCORE; //increment score

                *numBullets -= 1;
                *numMutants -= 1;

                i--;
                j--;

                hasCollided = TRUE; //set to true to indicate that a collision with this alien has already happened
            }
        }

        if(!hasCollided)
        {
            int dirX = 0; //used as direction values for any alien bullets if player in shoot radius
            int dirY = 0;

            if(DoIntersect(player->rect, mutants[i]->rect))
            {
                mvwprintw(playArea, mutants[i]->y, mutants[i]->x, mutants[i]->mutantEmpty); //draw blank space where they were

                CreateExplosion(mutants[i]->x, mutants[i]->y); 

                DestroyMutant(mutants[i]);
                ReorderMutantAlienArray(mutants, i, MAX_MUTANTS);
                *numMutants -= 1;

                PlayerHit(player, playerLives); //handles the player death

                hasCollided = TRUE;

            } else if(PlayerInMutantRadius(player, mutants[i], player->rect, mutants[i]->detectionRect, &dirX, &dirY)) //else if the player is in the aliens attack radius, shoot!
            {
                if(!mutants[i]->hasShot)
                {
                    if(*numAlienBullets < MAX_ALIEN_BULLETS)
                    {
                        mutants[i]->hasShot = TRUE;
                        alienBullets[*numAlienBullets] = NewAlienBullet(ALIEN_BULLET_SPEED, mutants[i]->x, mutants[i]->y, ALIEN_BULLET_LIFETIME, dirX, dirY);
                        *numAlienBullets += 1;
                    }

                }
            } 
        }

        hasCollided = FALSE;
    }
} 

bool AlienInSpace(int spawnX, int spawnY, AlienShip *aliens[], int *numAliens){ //Ensures spawn point isn't where an alien already is 

    for(int i = 0; i < *numAliens; i++){
        
         if((spawnX <= aliens[i]->x +5 && spawnX >= aliens[i]->x - 5) && (spawnY <= aliens[i]->y +3 && spawnY >= aliens[i]->y - 3))
         {
             return true;
         }
    }

    return false;
}

void SpawnAlien(int x, int y){
    CreateSpawnParticle(x,y);
}

void SpawningSystem(int *spawnTime, int *numAliensSpawned, PlayerShip *player, AlienShip *aliens[], int *numAliens, WINDOW *playArea, int minX, int maxX){

    *spawnTime -= 1; 
    int spawnAttempts = 0; //in order to prevent an infinite loop if the game can't find a place to spawn, it gets 4 attempts to try and find a spawn location

    if(*spawnTime <= 0)
    {
        *spawnTime = GenerateRandomBetween(MIN_SPAWN_TIME, MAX_SPAWN_TIME); //pick a random spawn time

        if((*numAliens +numParticles) < MAX_ALIENS) //if the maximum number of aliens + number of particles on the screen hasn't been (as those particles will soon be aliens)
        {
            //Spawn alien
            int spawnX, spawnY, xmax, ymax;
            int spawnPointValid = FALSE;

            getmaxyx(playArea, ymax, xmax);

            while(!spawnPointValid && spawnAttempts < 4){
                
                spawnX = GenerateRandomBetween(minX+3, maxX-3); //generate random spawn position within screen bounds
                spawnY = GenerateRandomBetween(3, ymax-5);

                if(!AlienInSpace(spawnX, spawnY, aliens, numAliens)) //ensure alien is not already in this space
                {
                    if((spawnX > player->x  +5 || spawnX < player->x - 5) && (spawnY > player->y  + 2 || spawnX < player->y - 2)) //ensure spawn point is not too close to player
                    {
                        spawnPointValid = TRUE;
                    }
                }

                spawnAttempts++;
            }

            SpawnAlien(spawnX, spawnY);
            *numAliensSpawned += 1;
        }
    }

}

void DestroyAllAliens(WINDOW *playArea, AlienShip *aliens[], int *numAliens){ //kills all of the aliens on screen 

    for(int i = 0; i < *numAliens; i++){
        mvwprintw(playArea, aliens[i]->y, aliens[i]->x, aliens[i]->alienTopEmpty);
        mvwprintw(playArea, aliens[i]->y+1, aliens[i]->x, aliens[i]->alienBottomEmpty);
        DestroyAlien(aliens[i]);
    }

    *numAliens = 0;
}

void DestroyAllMutants(WINDOW *playArea, MutantAlien *mutants[], int *numMutants){ //kills all of the mutants on screen

    for(int i = 0; i < *numMutants; i++){
        mvwprintw(playArea, mutants[i]->y, mutants[i]->x, mutants[i]->mutantEmpty);
        DestroyMutant(mutants[i]);
    }

    *numMutants = 0;
}

void DestroyAllAstronuats(WINDOW *playArea, Astronaut *astronuats[], int *numAstronauts){ //kill all astronuats that are falling/abducted or saved as the player has died

    for(int i = 0; i < *numAstronauts; i++){

        if(astronuats[i]->isFalling || astronuats[i]->isSaved || astronuats[i]->isAbducted || astronuats[i]->hasDied || astronuats[i]->y < astronuats[i]->bottomLevel){


            mvwprintw(playArea, astronuats[i]->y, astronuats[i]->x, astronuats[i]->personEmpty);
            DestroyAstronaut(astronuats[i]);
            ReorderAstronautArray(astronuats, i, MAX_ASTRONAUTS);

            *numAstronauts -= 1;
            i--;
        }

    }
}

void DestroyAllAlienBullets(WINDOW *playArea, AlienBullet *alienBullets[], int *numAlienBullets){ //removes all alien bullets

    for(int i = 0; i < *numAlienBullets; i++){
        mvwprintw(playArea, alienBullets[i]->y, alienBullets[i]->x, alienBullets[i]->bulletEmpty);
        DestroyAlienBullet(alienBullets[i]);
    }

    *numAlienBullets = 0;
}

void DestroyAllPlayerBullets(WINDOW *playArea, PlayerBullet *bullets[], int *numBullets){ //removes all bullets

    for(int i = 0; i < *numBullets; i++){
        mvwprintw(playArea, bullets[i]->y, bullets[i]->x, bullets[i]->bulletEmpty);
        DestroyBullet(bullets[i]);
    }

    *numBullets = 0;
}

void DrawTerrain(WINDOW *playArea, int screenMin, int screenMax, int maxY){

        char* topChar =     "       /----\\  ^^^^                 ";  //characters if the mountain background that will be looped along the bottom of the world
    char* upperMiddleChar = "      /  /\\  \\/    \\                ";
         char* middleChar = "     /  /  \\ /      \\   /-------\\   ";
    char* lowermiddleChar = "    /  /    \\/ /\\    \\ /         \\  ";
         char* bottomChar = "___/__/______\\/__\\/\\__\\___________\\_";

    int terrainLength = strlen(bottomChar);

    //draw empties

    for(int i = screenMin; i < screenMax; i++){ //this iterates through each space on the screen and draws the mountains continously until the end if reached

        int adjustedIndex = i;

        while(adjustedIndex < 0){ //ensures the adjusted index is above 0 just in case of a negative screenmin being given 
            adjustedIndex += terrainLength;
        }

        adjustedIndex %= terrainLength; //effectiley loops through the character array using the % operator


        wattron(playArea, COLOR_PAIR(TERRAIN_COLOR_PAIR)); //Everything is drawn 3 times on each third of the pad to create the illusion of seamless scrolling
        mvwaddch(playArea, maxY, i-screenMin, bottomChar[adjustedIndex]);
        mvwaddch(playArea, maxY-1, i-screenMin, lowermiddleChar[adjustedIndex]);
        mvwaddch(playArea, maxY-2, i-screenMin, middleChar[adjustedIndex]);
        mvwaddch(playArea, maxY-3, i-screenMin, upperMiddleChar[adjustedIndex]);
        mvwaddch(playArea, maxY-4, i-screenMin, topChar[adjustedIndex]);

        mvwaddch(playArea, maxY, i, bottomChar[adjustedIndex]);
        mvwaddch(playArea, maxY-1, i, lowermiddleChar[adjustedIndex]);
        mvwaddch(playArea, maxY-2, i, middleChar[adjustedIndex]);
        mvwaddch(playArea, maxY-3, i, upperMiddleChar[adjustedIndex]);
        mvwaddch(playArea, maxY-4, i, topChar[adjustedIndex]);

        mvwaddch(playArea, maxY, i+screenMin, bottomChar[adjustedIndex]);
        mvwaddch(playArea, maxY-1, i+screenMin, lowermiddleChar[adjustedIndex]);
        mvwaddch(playArea, maxY-2, i+screenMin, middleChar[adjustedIndex]);
        mvwaddch(playArea, maxY-3, i+screenMin, upperMiddleChar[adjustedIndex]);
        mvwaddch(playArea, maxY-4, i+screenMin, topChar[adjustedIndex]);
        wattroff(playArea, COLOR_PAIR(TERRAIN_COLOR_PAIR));
    }

}

void DrawSpawnParticles(WINDOW *playArea, AlienShip *aliens[], int *numAliens){
     int counter = 0;

       while(counter < numParticles){   //for each frame of the spawn particle draw all 3 rows of the particle
           
           mvwprintw(playArea, particles[counter]->y, particles[counter]->x, particles[counter]->empty);
           mvwprintw(playArea, particles[counter]->y +1, particles[counter]->x, particles[counter]->empty);
           mvwprintw(playArea, particles[counter]->y +2, particles[counter]->x, particles[counter]->empty);

           AnimateSpawnParticle(particles[counter]);

           if(particles[counter]->currentFrame == 1)
           {
               wattron(playArea, COLOR_PAIR(BULLET_COLOR_PAIR));
               mvwprintw(playArea, particles[counter]->y, particles[counter]->x, particles[counter]->frame1[0]);
               mvwprintw(playArea, particles[counter]->y+1, particles[counter]->x, particles[counter]->frame1[1]);
               mvwprintw(playArea, particles[counter]->y+2, particles[counter]->x, particles[counter]->frame1[2]);
               wattroff(playArea, COLOR_PAIR(BULLET_COLOR_PAIR));

           }else if(particles[counter]->currentFrame == 2)
           {
               wattron(playArea, COLOR_PAIR(BULLET_COLOR_PAIR));
               mvwprintw(playArea, particles[counter]->y, particles[counter]->x, particles[counter]->frame2[0]);
               mvwprintw(playArea, particles[counter]->y+1, particles[counter]->x, particles[counter]->frame2[1]);
               mvwprintw(playArea, particles[counter]->y+2, particles[counter]->x, particles[counter]->frame2[2]);
               wattroff(playArea, COLOR_PAIR(BULLET_COLOR_PAIR));

           }else if(particles[counter]->currentFrame == 3)
           {
               wattron(playArea, COLOR_PAIR(BULLET_COLOR_PAIR));
               mvwprintw(playArea, particles[counter]->y, particles[counter]->x, particles[counter]->frame3[0]);
               mvwprintw(playArea, particles[counter]->y+1, particles[counter]->x, particles[counter]->frame3[1]);
               mvwprintw(playArea, particles[counter]->y+2, particles[counter]->x, particles[counter]->frame3[2]);
               wattroff(playArea, COLOR_PAIR(BULLET_COLOR_PAIR));

               particles[counter]->frameCounter = 0;  //animation has ended so start it again and increment loop counter
               particles[counter]->loopCounter += 1;
               particles[counter]->currentFrame = 0;

           }else if(particles[counter]->loopCounter > 3) //when animation is done spawn an alien and destroy the particle
           {
               mvwprintw(playArea, particles[counter]->y, particles[counter]->x, particles[counter]->empty);
               mvwprintw(playArea, particles[counter]->y +1, particles[counter]->x, particles[counter]->empty);
               mvwprintw(playArea, particles[counter]->y +2, particles[counter]->x, particles[counter]->empty);
               
               if(*numAliens < MAX_ALIENS) //if the maximum number of aliens on the screen hasn't been 
                {
                    aliens[*numAliens] = NewAlien(particles[counter]->alienSpawnX, particles[counter]->alienSpawnY);
                    *numAliens += 1;
                }

               DestroySpawnParticle(particles[counter]);
               ReorderSpawnParticleArray(particles, counter, MAX_PARTICLES);
               numParticles--;
               counter--; 
           }       

        counter++; 
       }  
}

void DrawExplosions(WINDOW *playArea){
     int counter = 0;

       while(counter < numExplosions){   //for each frame of the explosion draw all 5 rows of the explosion
           
           mvwprintw(playArea, explosions[counter]->y, explosions[counter]->x, explosions[counter]->empty);
           mvwprintw(playArea, explosions[counter]->y +1, explosions[counter]->x, explosions[counter]->empty);
           mvwprintw(playArea, explosions[counter]->y +2, explosions[counter]->x, explosions[counter]->empty);
           mvwprintw(playArea, explosions[counter]->y +3, explosions[counter]->x, explosions[counter]->empty);
           mvwprintw(playArea, explosions[counter]->y +4, explosions[counter]->x, explosions[counter]->empty);

           AnimateExplosion(explosions[counter]);

           if(explosions[counter]->currentFrame == 1)
           {
               wattron(playArea, COLOR_PAIR(ALIEN_BULLET_COLOR_PAIR));
               mvwprintw(playArea, explosions[counter]->y, explosions[counter]->x, explosions[counter]->frame1[0]);
               mvwprintw(playArea, explosions[counter]->y +2, explosions[counter]->x, explosions[counter]->frame1[2]);
               mvwprintw(playArea, explosions[counter]->y +4, explosions[counter]->x, explosions[counter]->frame1[4]);
               wattroff(playArea, COLOR_PAIR(ALIEN_BULLET_COLOR_PAIR));

               wattron(playArea, COLOR_PAIR(MUTANT_ALIEN_COLOR_PAIR));
               mvwprintw(playArea, explosions[counter]->y +1, explosions[counter]->x, explosions[counter]->frame1[1]);
               mvwprintw(playArea, explosions[counter]->y +3, explosions[counter]->x, explosions[counter]->frame1[3]);
               wattroff(playArea, COLOR_PAIR(MUTANT_ALIEN_COLOR_PAIR));   

           }else if(explosions[counter]->currentFrame == 2)
           {
               wattron(playArea, COLOR_PAIR(ALIEN_BULLET_COLOR_PAIR));
               mvwprintw(playArea, explosions[counter]->y +4, explosions[counter]->x, explosions[counter]->frame2[4]);
               mvwprintw(playArea, explosions[counter]->y +1, explosions[counter]->x, explosions[counter]->frame2[1]);
               wattroff(playArea, COLOR_PAIR(ALIEN_BULLET_COLOR_PAIR));

               wattron(playArea, COLOR_PAIR(MUTANT_ALIEN_COLOR_PAIR));
               mvwprintw(playArea, explosions[counter]->y, explosions[counter]->x, explosions[counter]->frame2[0]);
               mvwprintw(playArea, explosions[counter]->y +2, explosions[counter]->x, explosions[counter]->frame2[2]);
               mvwprintw(playArea, explosions[counter]->y +3, explosions[counter]->x, explosions[counter]->frame2[3]);
               wattroff(playArea, COLOR_PAIR(MUTANT_ALIEN_COLOR_PAIR)); 

           }else if(explosions[counter]->currentFrame == 3)
           {
               wattron(playArea, COLOR_PAIR(ALIEN_BULLET_COLOR_PAIR));
               mvwprintw(playArea, explosions[counter]->y, explosions[counter]->x, explosions[counter]->frame3[0]);
               mvwprintw(playArea, explosions[counter]->y +2, explosions[counter]->x, explosions[counter]->frame3[2]);
               mvwprintw(playArea, explosions[counter]->y +4, explosions[counter]->x, explosions[counter]->frame3[4]);
               wattroff(playArea, COLOR_PAIR(ALIEN_BULLET_COLOR_PAIR));

               wattron(playArea, COLOR_PAIR(MUTANT_ALIEN_COLOR_PAIR));
               mvwprintw(playArea, explosions[counter]->y +1, explosions[counter]->x, explosions[counter]->frame3[1]);
               mvwprintw(playArea, explosions[counter]->y +3, explosions[counter]->x, explosions[counter]->frame3[3]);
               wattroff(playArea, COLOR_PAIR(MUTANT_ALIEN_COLOR_PAIR)); 

           }else if(explosions[counter]->currentFrame == 4)
           {
               wattron(playArea, COLOR_PAIR(ALIEN_BULLET_COLOR_PAIR));
               mvwprintw(playArea, explosions[counter]->y +4, explosions[counter]->x, explosions[counter]->frame4[4]);
               mvwprintw(playArea, explosions[counter]->y +1, explosions[counter]->x, explosions[counter]->frame4[1]);
               wattroff(playArea, COLOR_PAIR(ALIEN_BULLET_COLOR_PAIR));

               wattron(playArea, COLOR_PAIR(MUTANT_ALIEN_COLOR_PAIR));
               mvwprintw(playArea, explosions[counter]->y, explosions[counter]->x, explosions[counter]->frame4[0]);
               mvwprintw(playArea, explosions[counter]->y +2, explosions[counter]->x, explosions[counter]->frame4[2]);
               mvwprintw(playArea, explosions[counter]->y +3, explosions[counter]->x, explosions[counter]->frame4[3]);
               wattroff(playArea, COLOR_PAIR(MUTANT_ALIEN_COLOR_PAIR)); 

           }else if(explosions[counter]->currentFrame > 3)
           {

               DestroyExplosion(explosions[counter]);
               ReorderExplosionArray(explosions, counter, MAX_EXPLOSIONS);
               numExplosions--;
               counter--; 
           }       

        counter++; 
       }  
}

void DrawPlayerBullets(WINDOW *playArea, PlayerBullet *bullets[], int *numBullets){

    int counter = 0;

       while(counter < *numBullets){  

           mvwprintw(playArea, bullets[counter]->y, bullets[counter]->x, bullets[counter]->bulletEmpty);

           MoveBullet(bullets[counter]);

            if(bullets[counter]->lifetime <= 0)
           {
               DestroyBullet(bullets[counter]);
               ReorderBulletArray(bullets, counter, MAX_BULLETS);
               *numBullets -= 1;
               counter--; //decrease counter so that it will revisit this same index, as a different bullet will be stored there

            }else
            {
                wattron(playArea, COLOR_PAIR(BULLET_COLOR_PAIR));
                mvwprintw(playArea, bullets[counter]->y, bullets[counter]->x, bullets[counter]->bulletChar); //print bullet character at position y,x
                wattroff(playArea, COLOR_PAIR(BULLET_COLOR_PAIR));
            }          

        counter++; 
       }   
}

void DrawAlienBullets(WINDOW *playArea, AlienBullet *alienBullets[], int *numAlienBullets){

        int counter = 0;

        while(counter < *numAlienBullets){  

           mvwprintw(playArea, alienBullets[counter]->y, alienBullets[counter]->x, alienBullets[counter]->bulletEmpty);

            MoveAlienBullet(alienBullets[counter]);
            
            if(alienBullets[counter]->lifetime <= 0)
            {
                DestroyAlienBullet(alienBullets[counter]);
                ReorderAlienBulletArray(alienBullets, counter, MAX_ALIEN_BULLETS);
                *numAlienBullets -= 1;
                counter--; //decrease counter so that it will revisit this same index, as a different bullet will be stored there

                }else
                {
                    wattron(playArea, COLOR_PAIR(ALIEN_BULLET_COLOR_PAIR));
                    mvwprintw(playArea, alienBullets[counter]->y, alienBullets[counter]->x, alienBullets[counter]->bulletChar); //print bullet character at position y,x
                    wattroff(playArea, COLOR_PAIR(ALIEN_BULLET_COLOR_PAIR));
                }                

            counter++; 
        } 
}

//Everything is drawn 3 times on each third of the pad to create the illusion of seamless scrolling
void DrawAliens(WINDOW *playArea, AlienShip *aliens[], int *numAliens, int *screenMaxX, int *screenMaxY, int offset){//the offset is used to draw everything 3 times

       int counter = 0;

       while(counter < *numAliens){   
  
           mvwprintw(playArea, aliens[counter]->y, aliens[counter]->x, aliens[counter]->alienTopEmpty); //draw empty character to replace both rows of the alien character
           mvwprintw(playArea, (aliens[counter]->y) +1, aliens[counter]->x, aliens[counter]->alienBottomEmpty);

           mvwprintw(playArea, aliens[counter]->y, aliens[counter]->x - offset, aliens[counter]->alienTopEmpty); 
           mvwprintw(playArea, (aliens[counter]->y) +1, aliens[counter]->x - offset, aliens[counter]->alienBottomEmpty);

           mvwprintw(playArea, aliens[counter]->y, aliens[counter]->x + offset, aliens[counter]->alienTopEmpty); 
           mvwprintw(playArea, (aliens[counter]->y) +1, aliens[counter]->x + offset, aliens[counter]->alienBottomEmpty);

           if(aliens[counter]->hasAstronaut) //if alien has astronaut draw a blank space where the astronaut was
           { 
               mvwprintw(playArea, aliens[counter]->astronaut->y, aliens[counter]->astronaut->x, aliens[counter]->astronaut->personEmpty);  

               mvwprintw(playArea, aliens[counter]->astronaut->y, aliens[counter]->astronaut->x - offset, aliens[counter]->astronaut->personEmpty); 
               mvwprintw(playArea, aliens[counter]->astronaut->y, aliens[counter]->astronaut->x + offset, aliens[counter]->astronaut->personEmpty);                             
           }

           MoveAlien(aliens[counter], DELAYTIME, *screenMaxX, *screenMaxY, offset);
           
           wattron(playArea, COLOR_PAIR(ALIEN_COLOR_PAIR));
           mvwprintw(playArea, aliens[counter]->y, aliens[counter]->x, aliens[counter]->alienTop); //print top and bottom rows of alien character
           mvwprintw(playArea, aliens[counter]->y +1, aliens[counter]->x, aliens[counter]->alienBottom);

           mvwprintw(playArea, aliens[counter]->y, aliens[counter]->x - offset, aliens[counter]->alienTop); 
           mvwprintw(playArea, aliens[counter]->y +1, aliens[counter]->x - offset, aliens[counter]->alienBottom);

           mvwprintw(playArea, aliens[counter]->y, aliens[counter]->x + offset, aliens[counter]->alienTop); 
           mvwprintw(playArea, aliens[counter]->y +1, aliens[counter]->x + offset, aliens[counter]->alienBottom);

           wattroff(playArea, COLOR_PAIR(ALIEN_COLOR_PAIR));   

            wattron(playArea, COLOR_PAIR(ALIEN_BULLET_COLOR_PAIR));
            mvwprintw(playArea, aliens[counter]->y, aliens[counter]->x+2, "-"); //draws yellow eye/window of alien ship

            mvwprintw(playArea, aliens[counter]->y, (aliens[counter]->x -offset)+2, "-"); //draws yellow eye/window of alien ship
            mvwprintw(playArea, aliens[counter]->y, (aliens[counter]->x +offset)+2, "-"); //draws yellow eye/window of alien ship
            wattroff(playArea, COLOR_PAIR(ALIEN_BULLET_COLOR_PAIR));

           counter++; 
        }  

}

void DrawMutants(WINDOW *playArea, PlayerShip *player, MutantAlien *mutants[], int *numMutants, int *screenMaxX, int *screenMaxY, int offset){

    int counter = 0;

       while(counter < *numMutants){   
  
           mvwprintw(playArea, mutants[counter]->y, mutants[counter]->x, mutants[counter]->mutantEmpty); //draw empty character to replace both rows of the alien character
           
           mvwprintw(playArea, mutants[counter]->y, mutants[counter]->x - offset, mutants[counter]->mutantEmpty);
           mvwprintw(playArea, mutants[counter]->y, mutants[counter]->x + offset, mutants[counter]->mutantEmpty);

           MoveMutant(mutants[counter], player, DELAYTIME, *screenMaxX, *screenMaxY, offset);
           
           wattron(playArea, COLOR_PAIR(MUTANT_ALIEN_COLOR_PAIR));
           mvwprintw(playArea, mutants[counter]->y, mutants[counter]->x, mutants[counter]->mutantChar); //print mutant alien character

           mvwprintw(playArea, mutants[counter]->y, mutants[counter]->x - offset, mutants[counter]->mutantChar); //print mutant alien character
           mvwprintw(playArea, mutants[counter]->y, mutants[counter]->x + offset, mutants[counter]->mutantChar); //print mutant alien character
           wattroff(playArea, COLOR_PAIR(MUTANT_ALIEN_COLOR_PAIR));   

            wattron(playArea, COLOR_PAIR(ALIEN_BULLET_COLOR_PAIR));
            mvwprintw(playArea, mutants[counter]->y, mutants[counter]->x+1, "-"); //draws yellow eye/window of mutant alien
            mvwprintw(playArea, mutants[counter]->y, mutants[counter]->x+5, "-"); 

            mvwprintw(playArea, mutants[counter]->y, (mutants[counter]->x -offset)+1, "-"); //draws yellow eye/window of mutant alien
            mvwprintw(playArea, mutants[counter]->y, (mutants[counter]->x -offset)+5, "-"); 

            mvwprintw(playArea, mutants[counter]->y, (mutants[counter]->x +offset)+1, "-"); //draws yellow eye/window of mutant alien
            mvwprintw(playArea, mutants[counter]->y, (mutants[counter]->x +offset)+5, "-"); 
            wattroff(playArea, COLOR_PAIR(ALIEN_BULLET_COLOR_PAIR));

           counter++; 
        }  
}

void DrawAstronauts(WINDOW *playArea, Astronaut *astronauts[], int *numAstronauts, int offset){

    int counter = 0;

        while(counter < *numAstronauts){  

           mvwprintw(playArea, astronauts[counter]->y, astronauts[counter]->x, astronauts[counter]->personEmpty);

           mvwprintw(playArea, astronauts[counter]->y, astronauts[counter]->x - offset, astronauts[counter]->personEmpty);
           mvwprintw(playArea, astronauts[counter]->y, astronauts[counter]->x + offset, astronauts[counter]->personEmpty);

           MovePerson(astronauts[counter], DELAYTIME);

           wattron(playArea, COLOR_PAIR(PLAYER_COLOR_PAIR));
           mvwprintw(playArea, astronauts[counter]->y, astronauts[counter]->x, astronauts[counter]->personChar); //print astronaut character at position y,x

           mvwprintw(playArea, astronauts[counter]->y, astronauts[counter]->x -offset, astronauts[counter]->personChar); //print astronaut character at position y,x
           mvwprintw(playArea, astronauts[counter]->y, astronauts[counter]->x +offset, astronauts[counter]->personChar); //print astronaut character at position y,x
           wattroff(playArea, COLOR_PAIR(PLAYER_COLOR_PAIR));          

           counter++; 
        }  
}

void DrawPlayer(WINDOW *playArea, PlayerShip *player){

       wattron(playArea, COLOR_PAIR(PLAYER_COLOR_PAIR));
       mvwprintw(playArea, player->y, player->x, player->playerChar); //print player character at position y,x
       wattroff(playArea, COLOR_PAIR(PLAYER_COLOR_PAIR));
}

int main(){

   initscr();			// Start curses mode 	
   raw();				/* Line buffering disabled	*/	  
   keypad(stdscr, TRUE); //allows reading of keyboard keys
   nodelay(stdscr, TRUE); //means getch won't wait for user input
   noecho();			/* Don't echo() what we type*/
   curs_set(FALSE); // Don't display a cursor

   
   resizeterm(30, 120); //Set Dimensions of Terminal so that everyone has the same game experience

    //Make windows
    int max_y = 0, max_x = 0; //bounds of the whole screen   
    int screenMaxX = 0, screenMaxY = 0; //bounds of playing area

    getmaxyx(stdscr, max_y, max_x); //get the bounds of the screen 

    WINDOW* scoreArea = newwin(5, max_x, 0, 0);
    WINDOW* playArea = newpad(25, max_x * 3);

    getmaxyx(playArea, screenMaxY, screenMaxX); 
    screenMaxY -= 1;//deduct 2 from screen max X and Y in order to include the borders

    int mainScreenPos = max_x; //position of main screen where everything is happening

    int screenPos = mainScreenPos; //position of start of the viewable window
    int endScreenPos = (max_x*2) -1; //position of end of the viewable window 


    int frontEndPos = mainScreenPos - 1; //the x position of the front of the screen (how far player can move to the left before being teleported to the back)
    int backEndPos = endScreenPos + 1; //the x position of the end of the screen (how far player can move to the rigjt before being teleported to the front)

   //Check colours work in the terminal
    if (has_colors() == FALSE) 
    {
        endwin();
        printf("Your terminal does not support color\n");

        delwin(scoreArea); 
        delwin(playArea);
        exit(1);
    }

    start_color(); //Define colour codes
    init_pair(PLAYER_COLOR_PAIR, COLOR_CYAN, COLOR_BLACK);    
    init_pair(BULLET_COLOR_PAIR, COLOR_MAGENTA, COLOR_BLACK); 
    init_pair(ALIEN_COLOR_PAIR, COLOR_GREEN, COLOR_BLACK); 
    init_pair(ALIEN_BULLET_COLOR_PAIR, COLOR_YELLOW, COLOR_BLACK); 
    init_pair(MUTANT_ALIEN_COLOR_PAIR, COLOR_RED, COLOR_BLACK); 
    init_pair(TERRAIN_COLOR_PAIR, COLOR_RED, COLOR_BLACK);

    //Initialise vars
    int userInput; //variable which stores integer value of the character pressed on the keyboard
    int score = 0;

    int gameOver = FALSE;
    
    //Initialise player variables
    PlayerShip *player = NewPlayer(mainScreenPos + (max_x/4), PLAYER_SPAWN_Y);
    PlayerBullet *bullets[MAX_BULLETS];
    int playerLives = 6;

    int deathDelayTime = 50; //the tmie it takes for the player to respawn after death
    int deathDelayTimer = 0;

    int numBullets = 0;
    float thrustForceX = 0; //variables used to store force exerted from user input on player each game loop
    float thrustForceY = 0;

    //Initialise alien variables
    AlienShip *aliens[MAX_ALIENS];
    int numAliens = 0; //number of aliens current on the screen
    int numAliensSpawned = 0; //number of aliens spawned in this level to be used to have waves or levels with a certain number of aliens

    int spawnTimer = 0; //timer used to indicate when to next spawn an alien

    MutantAlien *mutants[MAX_MUTANTS];
    int numMutants = 0; //number of mutant aliens current on the screen

    AlienBullet *alienBullets[MAX_ALIEN_BULLETS];
    int numAlienBullets = 0;

    //Initialise astronaut variables
    Astronaut *astronauts[MAX_ASTRONAUTS];
    int numAstronauts = MAX_ASTRONAUTS;

    SetUpAstronauts(astronauts, playArea, screenPos+3, endScreenPos-3); //places astronauts on the level

    int i = 0;
 
   //Main game loop-------------------------------------------------------------------------------------------------------------------
   while(userInput != EXIT){ 

        werase(playArea); //clears screen so that everything can be repainted
        werase(scoreArea);

        DrawBorders(scoreArea);   //draw border for Game HUD  i.e. score display
        
        DrawSomeBorders(playArea);

        userInput = getch();      //get user input


        if(!player->hasDied && !gameOver){
                                                                               
            HandleUserInput(userInput, player, &thrustForceX, &thrustForceY, bullets, &numBullets);     //handle user input

            SpawningSystem(&spawnTimer, &numAliensSpawned, player, aliens,&numAliens, playArea, mainScreenPos, endScreenPos);

            //Implement Player Movement
            PlayerMove(player, thrustForceX, thrustForceY, DELAYTIME, screenMaxX, screenMaxY);          

            if(player->movingRight && (player->x + player->distX) > backEndPos) //if at end of screen teleport player to the front
            {
                int startDiff = player->x - screenPos;

                mvwprintw(playArea, player->y, player->x, player->playerEmpty);  
                player->x = mainScreenPos;

                screenPos = mainScreenPos - startDiff;
                endScreenPos = screenPos + mainScreenPos;

            }else if(!player->movingRight && (player->x + player->distX) < frontEndPos) //if at start of screen and moving left teleport player to end of screen
            {
                int endDiff = endScreenPos - player->x;
                int startdiff = mainScreenPos - player->x;

                mvwprintw(playArea, player->y, player->x, player->playerEmpty);  
                player->x = backEndPos - startdiff;

                endScreenPos = player->x + endDiff;
                screenPos = endScreenPos - max_x;
            }
            
            if((player->movingRight && abs(screenPos - (player->x + player->distX)) > 30) || (!player->movingRight && abs(endScreenPos - (player->x + player->distX)) > 30)) //scrolls the screen slightly when the player gets close to the borders of the visible area
            { 
                if(player->movingRight){
                    
                    screenPos +=  round(abs(screenPos - (player->x + player->distX)) * 0.1);
                    endScreenPos = screenPos + mainScreenPos;

                }else
                {

                    screenPos -=  round(abs(endScreenPos - (player->x + player->distX)) * 0.1);
                    endScreenPos = screenPos + mainScreenPos;
                }


            }else
            {
                screenPos += round(player->distX);
                endScreenPos += round(player->distX);   
            }

            //Draw other entities

            DrawTerrain(playArea, mainScreenPos, mainScreenPos+max_x, screenMaxY-1);

            DrawExplosions(playArea); //draw all the elements of the game

            DrawSpawnParticles(playArea, aliens, &numAliens);

            DrawPlayerBullets(playArea, bullets, &numBullets);

            DrawAliens(playArea, aliens, &numAliens, &endScreenPos, &screenMaxY, mainScreenPos);

            DrawMutants(playArea, player, mutants, &numMutants, &endScreenPos, &screenMaxY, mainScreenPos);

            DrawAlienBullets(playArea, alienBullets, &numAlienBullets);

            DrawAstronauts(playArea, astronauts, &numAstronauts, mainScreenPos);

            DrawPlayer(playArea, player);
            
            //Handle Collisions
            CheckAstronautState(player, aliens, &numAliens, astronauts, &numAstronauts, bullets, &numBullets, mutants, &numMutants, playArea, &gameOver); //checks the state of the astonauts i.e. if dead/mutated/saved/falling
            HandleMutantCollisions(player, &playerLives, bullets, &numBullets, alienBullets, &numAlienBullets, mutants, &numMutants, playArea, &score);
            CheckCollisions(player, &playerLives, bullets, &numBullets, alienBullets, &numAlienBullets, aliens, &numAliens, playArea, &score); //check collisions between aliens and bullets as well as the player and aliens

            //Draw GAME UI
            wattron(scoreArea, COLOR_PAIR(PLAYER_COLOR_PAIR));
            mvwprintw(scoreArea, 1, 5, "%s", player->playerRight); //display player icon
            wattroff(scoreArea, COLOR_PAIR(PLAYER_COLOR_PAIR));

            mvwprintw(scoreArea, 1, 5 + player->length, "x %d", playerLives); //display player lives
            mvwprintw(scoreArea, 3, 5, "%d", score); //display score

        }else{

            DrawExplosions(playArea); //draw explosions to animate player death explosion

            if(deathDelayTimer == 0) //delete all entities on the first loop of this death sequence
            {
                DestroyAllAliens(playArea, aliens, &numAliens);
                DestroyAllMutants(playArea, mutants, &numMutants);
                DestroyAllAstronuats(playArea, astronauts, &numAstronauts);
                DestroyAllAlienBullets(playArea, alienBullets, &numAlienBullets);
                DestroyAllPlayerBullets(playArea, bullets, &numBullets);

                if(playerLives <= 0)
                {
                    gameOver = TRUE;
                }
            }

            if(numExplosions == 0) //make screen go to black when the explosion is over
            {
                CleanScreen(playArea);
                CleanScreen(scoreArea);
                screenPos = mainScreenPos;
            }

            deathDelayTimer++;

            if(deathDelayTimer > deathDelayTime && !gameOver && numExplosions == 0)
            {
                player->hasDied = FALSE;
                deathDelayTimer = 0;
                ResetPlayer(mainScreenPos + (max_x/4), PLAYER_SPAWN_Y, player);

            }else if(gameOver)
            {
                mvwprintw(playArea, (max_y/4), mainScreenPos + (max_x/2) - 4, "GAME OVER"); //display game over text 
            }
        }

       wrefresh(scoreArea); //update screens
       prefresh(playArea, 0, screenPos, 5, 0, max_y, max_x-1);

       usleep(30000); //sleep for 30,000 microseconds 
   }

    free(player); //free memory
    delwin(scoreArea); 
    delwin(playArea);

    endwin(); //exit ncurses mode

    return 0;
}