# 319K
Embedded Systems<br/>
Lab 10<br/>

Space Invaders<br/>
Anvit Raju and Sarthak Gupta

This game is our take on a classic arcade game, Space Invaders, in which a player moves their spaceship to aim at and obliterate enemy ships before they reach the bottom of the screen. We will use the slide pot to strafe left and right for aiming at the enemies. We will also use two buttons: one for shooting at the ships that are remaining on the screen, and the other for pausing the game. The score will be calculated and displayed at the end of the game depending on the number of enemies shot with respect to different weightages depending on the type of enemy. Player wins if all the enemy ships are destroyed before they reach the bottom.

Features:<br/>
Two buttons: one to shoot at enemy and one to pause/play the game<br/>
Slide pot: to move ship left and right<br/>
Multiple enemy aliens will be sprites<br/>
Player’s ship is also a sprite<br/>
Moving sprites of bullets<br/>
Sounds: shooting sound<br/>
Score will be kept based on enemy aliens destroyed<br/>
Languages: English and Spanish<br/>

Requirements met:<br/>
Two buttons: shoot, pause/play<br/>
Slide pot: move left/right<br/>
3 different Sprites: player ship, enemy aliens, and bullets<br/>
Shooting sound created when user shoots<br/>
Three ISRs: one to shoot bullets (edge triggered interrupt), one to move the ship (periodic SysTick interrupt), and one to pause the game (periodic interrupt, highest priority <br/>
Score is displayed at the end of the game<br/>
Language options (English and Spanish) are displayed at beginning of game<br/>

Level 1- Very slow speed, Boss shoots bullets not very often<br/>
Level 2- Normal speed, Boss shoots bullets often<br/>
Level 3- Very fast speed, Boss shoots bullets very often<br/>
