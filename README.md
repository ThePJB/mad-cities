# Mad Cities
- press r to start again
- hold i to see per-region income
- hold lshift for time warp

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


## Rivers
 - pick a bunch of random high points and make them just go downhill
 - or do it for every point lol
 - so just have array for riverness
 - if local minima, lake

## Todos
 - have like archaeology, stuff from the 6 people that inhabited it
 - also $current_year
 - show faction incomes
 - distinct faction colours: maybe rotating by phi or something?
 - add biome view as well
 - cut off penalty? ---- definitely
  - if no neighbours price very cheap
 - attacking bonus if you have more neighbours than them? maybe
 - name cities
 - notable cities - draw non capital cities too

## Done
 - try different income distributions
 - time warp
 - make deterministic, remove dt or dt = 1/60 and also 60fps lock, time warp is 4 updates per frame
 - maybe have n_factions != n_cells
   - could have wild lands actually would be pretty cool
 - faction names

 ## Maybe todos
 - maybe increase the cost of taking over further away instead of or as well as the upkeep thing
 - maybe simulate population, output?
 - rivers, proper elevation, proper graphics could all be cool
 - make roads go through center of shared edge
  - make roads do something, destroyed between rivals, not just a fully connected mesh etc
  - try adding reasons to develop
  - moving of capitals
    - historical figure might move it
  - alliances
    - historical figures might forge them
  - weaken regions when they get captured
  - vary defense score
  - differentiate factions - defensiveness, aggression, propensity to invest in different infrastructure, etc
    - a historical figure might change factions numbers
   - less noisy, more interesting: maybe chain reactions and stuff
   - rivers
   - make 3d
    - roads would have to be isolines
   - could do domain warping
   - economy
 - battle simulation
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