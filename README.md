# Mad Cities
press r to start again
hold i to see per-region income

## Todos
 - get rid of chosen one and have rollover give informaiton about the faction instead
 - faction income view
 - try different income distributions
 - name factions and capitals
 - time warp
 - make deterministic, remove dt or dt = 1/60 and also 60fps lock, time warp is 4 updates per frame
 - maybe have n_factions != n_cells
   - could have wild lands actually would be pretty cool
 - add biome view as well
 - cut off penalty?
 - rivers, proper elevation, proper graphics could all be cool
 - make roads go through center of shared edge
 - speculative
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

remember that the simpler the better



ok so the plan is
 - simulation
   - have money builds over time
   - allow building of roads, cost proportional to distance. benefit = ?
 - simulate building roads and changing faction based on accessibility etc
 - cities also have production inputs and outputs, trade etc
 - see if simulation can be non retarded
 - e.g. a city will turn independent if its not got a nearby, strong power influencing it
 - add more terrain considerations like rivers etc

hmm making a game with a custom vector engine, like a space game, wouldnt that be cool

prebuilt highways might be spicy


---------------------

actual roads: isolines

money: per faction

todo dict, sometimes doesnt find key after resizing, am i zeroing properly etc

----
really get bucket is the main dict function, its an lvalue as well, and it might be empty or might not be
need to communicate that as well as the ptr because u want the ptr as lvalue still

theyre a bit more bare metal, you could save extra hashes if you want i.e the faction thing is already hashed so just look it up without hashing again. bug prone? maybe lol. but it could matter if it was a really really tight loop, like the chunk logic in my voxelgame for instance
----------------

now why i added? right to have money be a per faction thing.
oo i could draw a sorted list of dankest factions lool
so yes faction ID

i think i will also be updating per faction?
well I could keep doing it per site but they would always be traversed in a certain order therefore money get spent in a certain order
wait if i moved that out i didnt need hashmap, lol

----------------
it doing it shit again lmao

ok theres a pattern here, everyone is a neighbour of 0,1,2,3,4 etc


------
now enable them to buy each other
within cells interlinked


some edges are not makign roads (on the edges), maybe its because length-1 or something

biomes with properties re defensibility, productivity etc
factions could have compatibility e.g. seafaring etc
culture = adapting

ah yes maybe some kind of increased upkeep with distance
and that could vary with faction as well e.g. wide vs tall

interesting mutating things as expressions like in rust, with a method chain. lets them be const
and its just like this thing is this

-----
bug: holdout in top right corner lol

----------
todos
so tax on getting big might be quite interesting
roads: have there only be incentive to build to friends, and if built to friends then more income
destroy roads to enemies

make region income noise based instead of pure random
or even better discrete and obvious

output goes to 0 after changing hands and ramps back up

capital cities etc
output is like population * base value maybe

this is actually awesome. add names and introspection

---------
how to do gui
does it make sense to be like heres an object, use it to draw? and its got like the bounds etc
snowkoban in theory would be like its another thing on the scene graph
what about use dear imgui?

also r to re randomize?
maybe get rid of roads or make useful
more interesting world gen, what about just pure noise?
distribute income with noise, or even better some cool discrete thing?
or rollover borders
flanking bonus for captures?

one type to another type advantage? eg mountain bonus attacking down or mountain same to other mountain?
fortifying points?
civs could just have a tendency
blunting attacks instead of just cant afford
fuck areas that get repeatedly taken over, war-torn

could have treaties, non constant
factions themselves could have 'strength' to factor into conquest

also broke picking but also dont care. add pick to the rc thing

more output for water adjacency or something if port is developed

nonuniform distribution
guess you could have a noise function for point_density and then go through and cull
coastlines and stuff would be weird then


can u realloc nullptr? it would be nice if u didnt have to initialize vlas or if destroying them meant they were good 2 go
well i can also just add .clear methods