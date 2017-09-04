# Game0: Shepherd Dog

Game0: Shepherd Dog is the code for the game0 in the 15-466 f17 course. The starter code was developed by Jim McCann, and the additional code was created by Breeanna Ebert.

There are 5 white squares defined in the space representing sheep. All sheep will move randomly at the same speed. If the sheep collide with one another, both sheep will continue walking, but in opposite directions.

There is 1 black square representing the dog. The dog is directly controlled by the player's mouse. If a sheep collides with the dog, the sheep will move in the opposite direction.

The goal is to keep all sheep in the pen as long as possible. As time passes in the game, the difficulty increases, and the sheep walk faster.

To start or reset, click the game anywhere. 

## Requirements

 - modern C++ compiler
 - glm
 - libSDL2

On Linux or OSX these requirements should be available from your package manager without too much hassle.


### Building through Windows
 - Build through Developer Command Prompt for Virtual Studio
 - Find directory containing main.exe
```
  nmake -f Makefile.win
 ```

### Command line options
 - None

### Design Source
```
  http://graphics.cs.cmu.edu/courses/15-466-f17/game0-designs/hungyuc/
```