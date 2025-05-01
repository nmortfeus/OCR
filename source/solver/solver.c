#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/// Compare two characters, ignoring the case.
///
/// \param a The first character
/// \param b The second character
/// \return 1 if the characters are equal, ignoring the case, 0 otherwise
char compareChars(char a, char b)
{
    if(a >= 'a' && a <= 'z')
    {
        a = a - 'a' + 'A';
    }

    if(b >= 'a' && b <= 'z')
    {
        b = b - 'a' + 'A';
    }

    return a == b;
}


/// Attempt to find the given word in the grid starting from a specified position
/// and direction.
///
/// Checks if the word can be found starting from the initial column and row,
/// progressing in the specified row and column directions. If the word is found,
/// updates the output parameters with the column and row of the last character
/// of the word and returns 1. If the word is not found, returns 0 without
/// modifying the output parameters.
///
/// \param grid The grid of characters to search within.
/// \param width The width of the grid.
/// \param height The height of the grid.
/// \param word The word to find in the grid.
/// \param col The starting column for the search.
/// \param row The starting row for the search.
/// \param dir_r The row direction to progress in.
/// \param dir_c The column direction to progress in.
/// \param o_endCol Output parameter for the ending column of the found word.
/// \param o_endRow Output parameter for the ending row of the found word.
/// \return 1 if the word is found, 0 otherwise.
char trySolve(char** grid, size_t width, size_t height, char* word, size_t col,
        size_t row, char dir_r, char dir_c, size_t* o_endCol, size_t* o_endRow)
{
    size_t w_progress = 0;
    
    while(word[w_progress] && compareChars(grid[row][col], word[w_progress]))
    {
        ++w_progress;
        row += dir_r;
        col += dir_c;

        if(row >= height || col >= width)
        {
            break;
        }
    }

    if(!word[w_progress])
    {
        *o_endCol = col - dir_c;
        *o_endRow = row - dir_r;
        return 1;
    }

    return 0;
}


/// Explore all possible directions from the given starting position to find the word.
///
/// Tries all 8 possible directions (horizontal, vertical, and diagonal) from
/// the starting column and row. If the word is found in any direction, updates
/// the output end column and row with the position of the last character of the
/// found word and returns 1. If not found, returns 0 without modifying the output
/// parameters.
///
/// \param grid The grid of characters to search within.
/// \param width The width of the grid.
/// \param height The height of the grid.
/// \param word The word to find in the grid.
/// \param startCol The starting column for the search.
/// \param startRow The starting row for the search.
/// \param o_endCol Output parameter for the ending column of the found word.
/// \param o_endRow Output parameter for the ending row of the found word.
/// \return 1 if the word is found, 0 otherwise.
char explore(char** grid, size_t width, size_t height, char* word,
        size_t startCol, size_t startRow,
        size_t* o_endCol, size_t* o_endRow)
{
    for(char dir_r = -1; dir_r <= 1; dir_r++)
    {
        for(char dir_c = -1; dir_c <= 1; dir_c++)
        {
            if((dir_r != 0 || dir_c != 0) &&
                    trySolve(grid, width, height, word, startCol, startRow,
                    dir_r, dir_c, o_endCol, o_endRow))
            {
                return 1;
            }
        }
    }

    return 0;
}


/// Attempt to find the given word in the grid, starting from each possible
/// position.
///
/// Iterates over the entire grid and, for each position, checks if the word
/// can be found starting from that position in any direction. If found, updates
/// the starting and ending positions of the word in the grid and returns 1. If
/// the word cannot be found in the grid, returns 0 without modifying output
/// parameters.
///
/// \param grid The grid of characters to search within.
/// \param width The width of the grid.
/// \param height The height of the grid.
/// \param word The word to find in the grid.
/// \param o_startCol Output parameter for the starting column of the found word.
/// \param o_startRow Output parameter for the starting row of the found word.
/// \param o_endCol Output parameter for the ending column of the found word.
/// \param o_endRow Output parameter for the ending row of the found word.
/// \return 1 if the word is found, 0 otherwise.
char solve(char**grid, size_t width, size_t height, char* word,
        size_t* o_startCol, size_t* o_startRow, size_t* o_endCol,
        size_t* o_endRow)
{
    // find the start letter
    
    for(size_t r = 0; r < height; r++)
    {
        for(size_t c = 0; c < width; c++)
        {
            if(compareChars(grid[r][c], word[0]) && explore(grid, width,
                    height, word, c, r, o_endCol, o_endRow))
            {
                *o_startCol = c;
                *o_startRow = r;
                return 1;
            }
        }
    }

    return 0;
}

void process_solve(char **grid, char **wordlist, int nb_grid_letters, int col, int len_words, int ***found){
    int found_len = 0;
    for(int i = 0; i < len_words-1; i++){
        char *word = wordlist[i];
        size_t o_startCol = 0, o_startRow = 0, o_endCol = 0, o_endRow = 0;
        if(solve(grid,col,nb_grid_letters/col,word,&o_startCol,&o_startRow,&o_endCol,&o_endRow) == 1){
            int *to_add = malloc(4*sizeof(int));
            to_add[0] = o_startCol;
            to_add[1] = o_startRow;
            to_add[2] = o_endCol;
            to_add[3] = o_endRow;
            *found = realloc(*found,(found_len+1)*sizeof(int *));
            (*found)[found_len] = to_add;
            found_len++;
            printf("Word %s found in x1=%zu y1=%zu x2=%zu y2=%zu\n",word,o_startCol,o_startRow,o_endCol,o_endRow);
        }
    }
    *found = realloc(*found,(found_len+1)*sizeof(int *));
    (*found)[found_len] = NULL;
}