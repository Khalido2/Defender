//Bullet STRUCTS AND ASSOCIATED FUNCTIONS 
//---------------------------------------------------------------------------------------------------------------------------------------------

#ifndef BULLET_H
#define BULLET_H

#include "collisions.h"

//Player Bullet STRUCT AND ASSOCIATED FUNCTIONS
//---------------------------------------------------------------------------------------------------------------------------------------------
typedef struct PlayerBullet{

    char bulletChar[3];
    char bulletEmpty[3];
    float speed;
    int lifetime; //the number of loops the bullet will survive if it doesn't collide with anything

    int x;
    int y;

    int direction;

    Rectangle *rect;

}PlayerBullet ;

PlayerBullet* NewBullet(float moveSpeed, int xPos, int yPos, int life, int dir){
    PlayerBullet *bullet = malloc(sizeof(PlayerBullet));

    char tempChar[] = "---";
    char tempEmpty[] = "   ";
    strcpy(bullet->bulletChar, tempChar);
    strcpy(bullet->bulletEmpty, tempEmpty);

    bullet->rect = NewRectangle(xPos,yPos, 3, 1);

    bullet->speed = moveSpeed;
    bullet->x = xPos;
    bullet->y = yPos;

    bullet->direction = dir;

    bullet->lifetime = life;

    return bullet;

}

void DestroyBullet(PlayerBullet *bullet){
    free(bullet);
}

void MoveBullet(PlayerBullet *bullet){ //direction is 1 for right and -1 for left
    bullet->x += bullet->speed * bullet->direction;
    bullet->lifetime--;
    ResetBounds(bullet->rect, bullet->x, bullet->y, 3, 1);
}

//reorders the bullet array to ensure active bullets are stored at the front end of the array with no gaps
void ReorderBulletArray(PlayerBullet *arr[], int index, int maxBullets){

    if(index == maxBullets-1)  //if the destroyed bullet was at the end of the array do nothing
    { 
        return;
    }

    for(int i = index+1; i < maxBullets; i++){
        arr[i-1] = arr[i]; 
    }
}



//Alien Bullet STRUCT AND ASSOCIATED FUNCTIONS
//---------------------------------------------------------------------------------------------------------------------------------------------
typedef struct AlienBullet{

    char bulletChar[3];
    char bulletEmpty[3];
    float speed;
    int lifetime; //the number of loops the bullet will survive if it doesn't collide with anything

    int timeBetweenMoves; //number of cycles between moves used to slow bullet down
    int timeCounter;

    int x;
    int y;

    int directionX;
    int directionY;

    Rectangle *rect;

}AlienBullet ;

AlienBullet* NewAlienBullet(float moveSpeed, int xPos, int yPos, int life, int dirX, int dirY){
    AlienBullet *bullet = malloc(sizeof(AlienBullet));

    char tempChar[] = "O";
    char tempEmpty[] = " ";
    strcpy(bullet->bulletChar, tempChar);
    strcpy(bullet->bulletEmpty, tempEmpty);

    bullet->rect = NewRectangle(xPos,yPos, 1, 1);

    bullet->speed = moveSpeed;

    bullet->directionX = dirX;
    bullet->directionY = dirY;

    bullet->timeBetweenMoves = 2;
    bullet->timeCounter = 0;

    bullet->x = xPos + bullet->timeBetweenMoves * dirX;
    bullet->y = yPos + bullet->timeBetweenMoves * dirY;

    bullet->lifetime = life;


    return bullet;

}

void DestroyAlienBullet(AlienBullet *bullet){
    free(bullet);
}

void MoveAlienBullet(AlienBullet *bullet){ //direction is 1 for right and -1 for left

    bullet->timeCounter += 1;

    if(bullet->timeCounter >= bullet->timeBetweenMoves)
    {
        bullet->timeCounter = 0;
        bullet->x += bullet->speed * bullet->directionX;
        bullet->y += bullet->speed * bullet->directionY;
        bullet->lifetime--;

        ResetBounds(bullet->rect, bullet->x, bullet->y, 1, 1);
    }

}

//reorders the bullet array to ensure active bullets are stored at the front end of the array with no gaps
void ReorderAlienBulletArray(AlienBullet *arr[], int index, int maxBullets){

    if(index == maxBullets-1)  //if the destroyed bullet was at the end of the array do nothing
    { 
        return;
    }

    for(int i = index+1; i < maxBullets; i++){
        arr[i-1] = arr[i]; 
    }
}

#endif