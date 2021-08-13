#include "minesweeper_game.h"

#include "display.h"
#include "tile.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct MinesweeperGame newMinesweeperGame(struct Pos size, int mines)
{
    struct MinesweeperGame game;

    game.status = PROGRESS;
    game.hasOpenedFirstTile = false;
    game.size = size;
    game.mines = mines;

    int field_area = size.x * size.y;

    game.tiles = malloc(sizeof(struct Tile) * field_area);

    for (int i = 0; i < field_area; i++)
    {
        struct Tile *tile = &game.tiles[i];
        tile->status = COVERED;
        tile->hasMine = false;
    }

    return game;
}

void freeMinesweeperGame(struct MinesweeperGame *game)
{
    free(game->tiles);
}

bool isInBound(struct Pos gameSize, struct Pos pos)
{
    return (pos.x >= 0 && pos.x < gameSize.x) && (pos.y >= 0 && pos.y < gameSize.y);
}

struct Tile *getTile(const struct MinesweeperGame *game, struct Pos pos)
{
    return &game->tiles[pos.x + pos.y * game->size.x];
}

void openTile(struct MinesweeperGame *game, struct Pos pos)
{
    struct Tile *tile = getTile(game, pos);

    tile->status = OPEN;

    if (tile->hasMine || tile->neighbours > 0)
        return;

    for (int y = -1; y <= 1; y++)
    {
        for (int x = -1; x <= 1; x++)
        {
            struct Pos neighbourPos = newPos(pos.x + x, pos.y + y);

            if (isInBound(game->size, neighbourPos) && getTile(game, neighbourPos)->status != OPEN)
                openTile(game, neighbourPos);

        }
    }
}

void insertMines(struct MinesweeperGame *game)
{
    int insertedMines = 0;

    for (int i = 0; i < game->size.x * game->size.y; i++)
    {
        struct Tile *tile = &game->tiles[i];

        if (insertedMines == game->mines)
            return;
        else if (tile->status == OPEN)
            continue;

        tile->hasMine = true;
        insertedMines++;
    }
}

void swapTiles(struct MinesweeperGame *game)
{
    for (int i = 0; i < game->size.x * game->size.y; i++)
    {
        struct Tile *currentTile = &game->tiles[i];

        if (currentTile->status == OPEN)
            continue;

        int randomIndex = randomInt(0, game->size.x * game->size.y - 1);

        struct Tile *randomTile = &game->tiles[randomIndex];

        bool isDifferentTile = i != randomIndex;
        bool isRandomTileNotOpen = randomTile->status != OPEN;

        if (isDifferentTile && isRandomTileNotOpen)
        {
            struct Tile temp = *currentTile;
            *currentTile = *randomTile;
            *randomTile = temp;
        }
    }
}

int getNumNeighoursMines(struct MinesweeperGame *game, struct Pos pos)
{
    int total = 0;

    for (int y = -1; y <= 1; y++)
    {
        for (int x = -1; x <= 1; x++)
        {
            if (x == 0 && y == 0)
                continue;

            struct Pos neighbourPos = newPos(pos.x + x, pos.y + y);

            if (isInBound(game->size, neighbourPos))
                total += getTile(game, neighbourPos)->hasMine;
        }
    }

    return total;
}

void setNeighboursMinefield(struct MinesweeperGame *game)
{
    for (int y = 0; y < game->size.y; y++)
    {
        for (int x = 0; x < game->size.x; x++)
        {
            struct Pos pos = newPos(x, y);
            getTile(game, pos)->neighbours = getNumNeighoursMines(game, pos);
        }
    }
}

void initMines(struct MinesweeperGame *game)
{
    insertMines(game);
    swapTiles(game);
    setNeighboursMinefield(game);
}

void openFirstTile(struct MinesweeperGame *game, struct Pos pos)
{
    getTile(game, pos)->status = OPEN;

    initMines(game);
    openTile(game, pos);
}

void doAction(struct Action action, struct MinesweeperGame *game)
{
    struct Tile *tile = getTile(game, action.pos);

    if (action.type == OPEN_TILE_ACTION)
    {
        if (game->hasOpenedFirstTile)
        {
            openTile(game, action.pos);
            if (tile->hasMine)
                game->status = LOST;
        }
        else
        {
            openFirstTile(game, action.pos);
            game->hasOpenedFirstTile = true;
        }
    }
    else if (tile->status != OPEN)
    {
        if (action.type == FLAG_TILE_ACTION)
            tile->status = FLAG;
        else if (action.type == GUESS_TILE_ACTION)
            tile->status = GUESS;
    }
}

struct Action getAction(struct MinesweeperGame *game)
{
    bool inputIsValid = true;
    struct Action action;
    action.pos = (struct Pos){-1, -1};

    char line[15];

    do
    {
        printf("> ");

        scanf("%[^\n]%*c", line);

        if (sscanf(line, "f %i %i", &action.pos.x, &action.pos.y) == 2 && isInBound(action.pos, game->size))
            action.type = FLAG_TILE_ACTION;
        else if (sscanf(line, "g %i %i", &action.pos.x, &action.pos.y) == 2 && isInBound(game->size, action.pos))
            action.type = GUESS_TILE_ACTION;
        else if (sscanf(line, "%i %i", &action.pos.x, &action.pos.y) == 2 && isInBound(game->size, action.pos))
            action.type = OPEN_TILE_ACTION;
        else
            inputIsValid = false;
    } while (!inputIsValid);

    return action;
}

void playGame(struct MinesweeperGame *game)
{
    const char *WON_TEXT = "Congratulations, you won!\n";
    const char *LOST_TEXT = "Sorry, you lost.\n";

    while (game->status == PROGRESS)
    {
        displayMinesweeperGame(game);

        doAction(getAction(game), game);
    }

    displayOpenMinesweeperGame(game);

    if (game->status == WON)
        printf(WON_TEXT);
    else
        printf(LOST_TEXT);
}
