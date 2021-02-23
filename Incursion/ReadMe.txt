SD1-A6 Incursion Alpha
Author: Yixuan Wei
SMU ID: 48085379

Brief: this project aims to build up a tank traveling and shooting to reach the 
	end of each level.

Known Issues: 
    1. When physics is disabled, and the player's center is inside some solid
	block, the player couldn't be pushed out when the physics is enabled 
	again. This is the cause of current buggy physics implementation.
    2. When raycasting, it is possible that the raycast route squeezes through 
	the interval of two diagonally adjacent solid tiles.
	So that you will see the AI tank stupidly stuck on the diagonally adjacent 
	tiles.
    3. It is possible that you will be pushed into solid tiles's adjacent intervals,
	thus going through the wall.
    4. Sometimes you can see tile boundaries in view.

How to Use:
--  For PROGRAMMERS: 
    This project contains mainly two parts: Engine and Incursion.
	Some detailed explanations are already in assignment instruction docx file.
	Double click on Incursion.sln inside Incursion to open the project in VS. Remember 
    to add $(SolutionDir)Run to Working Directory under Debugging of Project Property, so 
    that you could run debugger inside VS.
	Many debug usage are available. Whenever press F8, the game would simply reboot.
    When inside playing mode, press F1 to activate debug mode drawing, press F3 to toggle 
    physics system on and off, press F4 to toggle displaying the full map camera and play 
    mode camera, press Y to speed up to 4X of original fps, press T to slow down to 1/10th 
    of original fps, press N to spawn new friendly tanks and turrets.
--  For USERS: 
    1. When application is launched, the game starts with attract mode. Press spacebar on 
	keyboard or start button on xbox controller to start.
    2. There are three levels in total. Each filled with enemy turrets, enemy tanks, 
	boulders, default ground, stuck ground, solid walls and ONE end point on the top right 
	corner. Stuck ground slowers your speed, solid walls stops enemies' views and your 
	movement, boulders could deflect bullets, enemies would shoot you when having 
	discovered you. 
    3. After each tank or turret is dead, one loot will drop: either a heart to cure health or 
	a faction bomb to switch enemies in an area to become friendly. Notice that only the 
	loots dropped by enemies would take effect on you and your friends. You friendly tanks 
	or turrets would leave behind loots that is the same beneficial to the enemy faction.
    4. You always start from the bottom left corner of the map, use the xbox 
	controller to control the player. Left joystick for turning and moving. Right 
	joystick for turning the gun. Button A to shoot bullet. Button B to shoot faction bomb.
	Press key P on keyboard or start button would pause the game.
	Progress to the end point to win each level!

Deep Learning:
	Arrange better for the time schedule is always a hard task. I assumed some tasks to be
   	extremely easy, like AI tanks. But it turned out ot cost more than 3 hours for me 
	to refine. This is a similar topic for "Pick Your Battle". I used to be stuck on 
	fascinating tweaking stuff, and found it so hard to give up the progress I had 
	reached. I would start setting calender events (like what I did for scrum board in TGP1) 
	to help me better arrange my workflow.
