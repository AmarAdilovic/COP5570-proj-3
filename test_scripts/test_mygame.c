#include "myserver.h"

Game *game_head = NULL;

int main(int argc, char * argv[]) {
    char buffer[10];
    Game *test_game = create_game("hello", "dmm");
    while (isWin(test_game) == 0) {
        printf("%s\n", print_board(test_game));
        printf("Enter next move: \n");
        scanf("%s", buffer);

        int result = move(buffer, test_game);

        if (result == -1) {
            printf("Place is already taken \n");
        } else if (result == -2) {
            printf("Input is not in right format \n");
        } else if (result == 1) {
            printf("white win \n");
        } else if (result == 2) {
            printf("Black win \n");
        } else if (result == 3) {
            printf("Draw\n");
        }

        printf("Test: %d\n", result);
    }
    return 0;
}