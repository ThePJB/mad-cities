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