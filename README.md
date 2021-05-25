# Mad Cities
- press r to generate a new world
- hold lshift for time warp
- overlays - hold f, h, e, l, b, d

## Aims
 - be as simple as possible
 - have enough going on that the player fills in the gaps
 - to that end show enough varied stuff and name it etc
 - discrete, named (enum) > continuous
 - can use dict as doubledict with my own hashing, omg brilliant. so order wont matter.
 - so for each point, let rf = amount of rainfall
 - figure out down
 - while down, + rf
 - if local minima, make a lake. isoline?

## Todos
 - have like archaeology, remnants from the different factions that have controlled the city
 - distinct faction colours: maybe rotating by phi or something? doesnt seem to be perfect
 - expand cut off penalty to be if you cant path back to main
 - attacking bonus if you have more neighbours than them? maybe
 - maybe like faction inspector, city inspector, region inspector
 - 3d? maybe but dont get sidetracked lmao
 - name / grow cities? city simulation? development simulation - roads etc, how
 - instability consequences
  - move capital
  - assassinate leader
  - overthrow faction
  - breakaway faction - derived name (independent new thing of same last thing) (faction names, use index plz)

  - alliances
    - historical figures might forge them
  - weaken regions when they get captured
  - vary defense score
  - differentiate factions - defensiveness, aggression, propensity to invest in different infrastructure, etc
    - a historical figure might change factions numbers
   - less noisy, more interesting: maybe chain reactions and stuff

  - world gen?
   - need to solve local minimas first
   - then could go domain warping etc

 - seafaring
 - notable leaders
  - statesman
    - relocate capital
    - forge alliance
    - change policy, multipliers etc
      - e.g. expansionist, reduce penalty, only to have them collapse when person dies lol (after certain number of years)
    - reforms for increasing income etc
    - split
  - general
    - multiplier to combat effectiveness
  - proclamations
    - this nation bad
    - fear me
  - get assassinated
  - dynasty - old mate IV etc
 - more biomes, defensibility score, productivity score, etc
 - faction differences
   - wide vs tall
   - propensity to do certain things eg build road, conquer
 - decisive collapses
 - output goes to 0 after changing hands and ramps back up or its lioke -1 to size or something
 - size hamlet, village, town, city, capital city etc
 - flanking bonus for captures?
 - fortification? project power to neighbouring areas too?

civ multiplier for terrain type

one type to another type advantage? eg mountain bonus attacking down or mountain same to other mountain?
fortifying points?
civs could just have a tendency
blunting attacks instead of just cant afford
fuck areas that get repeatedly taken over, war-torn

could have treaties, non constant
factions themselves could have 'strength' to factor into conquest

more output for water adjacency or something if port is developed

got to have succession crisis if no heir lol


have archaeology and stuff from when cities have changed hands
history of city

------------------------

faces, edges look correct
vert positions potensh correct

with higher point counts we are getting invariant failures where edges only have one vertex. how strange.
and also segfaults
and also drawing wrong
so maybe this is a wrong assumption + game logic is wrong

next step: just try drawing a simple VD faces and edges. then you'll be able to see the offending degenerates as well.

i guess the fix is to literally go in and deliberately degenerate the edges

i think ive got to fix a bug ive already fixed before, its neighour counts being wrong, i guess because of having jank vertices

need to show edge dir view, also maybe height mode

--------------------------------------------------------

interesting income is always the same function lol

its definitely doing rivers in the wrong places.
they shouldnt be in rivers and they shouldnt be multiple from the highest node

maybe get lowest edge is wrong, need to select verts
lowest edge retarded

--------------

ok so there are duplicate points, need to round floating point to a certain interval thing, maybe 3 d.p. can also maybe use double for jcv

-------------------
some retarded stuff happening with rivers.
  - ok ocean detect just straight up does not work
  - some non adjacent edges lmao
  - some instability, infinite loops happen for some reason

maybe I should just make a set iter idk

ok it is doing something, the neighbour info must be r8 fucked
ok well as far as neighbour biomes go it looks correct from main debug stuff lmao

so the one that starts there terminates but the other ones keep going for some reason lmao
visiting a loop just involves 

infinite loop: theres a vertex (844) It just visits itself continuously

domain warping might make this a lot cooler

----------

maybe the stupid edge thing is literally just in drawing edges, doesnt hapen with faces
it does happen with the selecty thing
idk why it happens with that and not with normal faces
actually faces must do it but just get drawn over
the thing never goes up. so it must be 2x as common

no way it was actually stupid hashing

----------

todo local minima makes lakes. basically need like a dam algorithm and keep flowing out til u hit the sea (or really cant lol)

faction AI? maybe save money for a campaign and like blitz


-----------------------

todos:
 - colours for biomes
 - biome overlay
 - incomes probs wrong lmao
 - defence overlay

--------

could have mineral patches that are worth income too

defensive strat: just sit and rebuy


--------------

how to optimize the colours lol
add historical figures
have them get assassinated, have succession crisis and splits and stuff
how to calculate split points

-----------------
now just make leader stats factor in
todo make title line up with what they are
and maybe display leader number title

------
mega instability when money is negative or whatever
assassinating the leader, faction overthrow, breakaways from certain towns
death reasons