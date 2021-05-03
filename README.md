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