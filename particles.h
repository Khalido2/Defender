//Spawn particle STRUCT AND ASSOCIATED FUNCTIONS
//---------------------------------------------------------------------------------------------------------------------------------------------

#ifndef PARTICLES_H
#define PARTICLES_H

typedef struct SpawnParticle{

//4 char arrays to store characters of the 3 frames of the spawn particle plus one for the empty space

    char empty[11];
    char frame1[3][11];
    char frame2[3][11];
    char frame3[3][11];

    int currentFrame;

    int x,y, alienSpawnX, alienSpawnY;

    int timeBetweenFrames; //number of game loops between each frame
    int frameCounter;

    int loopCounter; //number of times the animation has been run through, as the animation will play 3 times

}SpawnParticle ;

void DestroySpawnParticle(SpawnParticle *particle){
    free(particle);
}

SpawnParticle* NewSpawnParticle(int x, int y){
    SpawnParticle *particle = malloc(sizeof(SpawnParticle));

    char empty[11] = "          ";
    char frame1[3][11] = {"   \\ /   ", "          ", "   / \\   "};
    char frame2[3][11] = {"  \\   /  ", "          ", "  /   \\  "};
    char frame3[3][11] = {"\\       /", "          ", "/       \\"};

    strcpy(particle->empty, empty);

    for(int i = 0; i < 3; i++){
        strcpy(particle->frame1[i], frame1[i]);
        strcpy(particle->frame2[i], frame2[i]);
        strcpy(particle->frame3[i], frame3[i]);
    }

    particle->timeBetweenFrames = 2;
    particle->frameCounter = 0;
    particle->currentFrame = 0;
    particle->loopCounter = 0;

    particle->x = x-4; //position is offset so x and y is in the middle of the particle
    particle->y = y-1;

    particle->alienSpawnX = x; //store spawn position where alien will be spawned
    particle->alienSpawnY = y;

    return particle;
}

void AnimateSpawnParticle(SpawnParticle *particle){  //using time delay increment frame counter to draw the different frames

    particle->frameCounter += 1;

    int frame = particle->frameCounter / particle->timeBetweenFrames;

    if(particle->frameCounter % particle->timeBetweenFrames == 0)
    {
        particle->currentFrame = frame;      
    }

}

//reorders the spawn particle array to ensure active particles are stored at the front end of the array with no gaps
void ReorderSpawnParticleArray(SpawnParticle *arr[], int index, int maxParticles){

    if(index == maxParticles-1)  //if the destroyed particle was at the end of the array do nothing
    { 
        return;
    }

    for(int i = index+1; i < maxParticles; i++){
        arr[i-1] = arr[i]; 
    }
}

#endif