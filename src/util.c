#include <stdlib.h>
#include "util.h"

int randomInt(int min, int max)
{
    return ((double)rand() / RAND_MAX) * (max - min + 1) + min;
}

_Bool isInBoundPos(Pos game_size, Pos pos)
{
    return pos.x < game_size.x && pos.y < game_size.y;
}
