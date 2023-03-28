//Explosion STRUCT AND ASSOCIATED FUNCTIONS
//---------------------------------------------------------------------------------------------------------------------------------------------

#ifndef EXPLOSION_H
#define EXPLOSION_H

typedef struct Explosion{

//5 char arrays to store characters of the 4 frames of the explosion plus one for the empty space

    char empty[8];
    char frame1[5][8];
    char frame2[5][8];
    char frame3[5][8];
    char frame4[5][8];

    int currentFrame;

    int x,y;

    int timeBetweenFrames; //number of game loops between each frame
    int frameCounter;

}Explosion ;

void DestroyExplosion(Explosion *explosion){
    free(explosion);
}

Explosion* NewExplosion(int x, int y){
    Explosion *explosion = malloc(sizeof(Explosion));

    char empty[8] = "       ";
    char frame1[5][8] = {"  -.-  ", "  #@#  ", ".#@#@#.", "  -@#  ", "  -.-  "};
    char frame2[5][8] = {"*-@#*--", " -#)-  ", ")-#!!* ", " -+ #* ", "  *_*  "};
    char frame3[5][8] = {"   .   ", "  #@   ", " .#!@# ", " @##.  ", "   -   "};
    char frame4[5][8] = {"       ", "  -@#  ","  #@.  ","   .-  ", "       "};

    strcpy(explosion->empty, empty);

    for(int i = 0; i < 5; i++){
        strcpy(explosion->frame1[i], frame1[i]);
        strcpy(explosion->frame2[i], frame2[i]);
        strcpy(explosion->frame3[i], frame3[i]);
        strcpy(explosion->frame4[i], frame4[i]);
    }

    explosion->timeBetweenFrames = 2;
    explosion->frameCounter = 0;
    explosion->currentFrame = 0;

    explosion->x = x+3; //position is offset so x and y is in the middle of the particle
    explosion->y = y-2;

    return explosion;
}

void AnimateExplosion(Explosion *explosion){ //using time delay increment frame counter to draw the different frames

    explosion->frameCounter += 1;

    int frame = explosion->frameCounter / explosion->timeBetweenFrames;

    if(explosion->frameCounter % explosion->timeBetweenFrames == 0)
    {
        explosion->currentFrame = frame;
    }

}

//reorders the explosion array to ensure active explosions are stored at the front end of the array with no gaps
void ReorderExplosionArray(Explosion *arr[], int index, int maxExplosions){

    if(index == maxExplosions-1)  //if the destroyed explosion was at the end of the array do nothing
    { 
        return;
    }

    for(int i = index+1; i < maxExplosions; i++){
        arr[i-1] = arr[i]; 
    }
}

#endif