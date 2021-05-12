so I want delauney, i.e. i want to know what the edges connect to
need to know for a cell the neighbouring cells


need to know:
 - for an edge the outgoing edges
 - for an edge the neighbouring faces
 - for a face the adjacent faces
 - for a face the edges

 https://www.programiz.com/dsa/graph-adjacency-list
 might want an adjacency list
 (double dutch backwards adjacancy list included)

 voronoi site = delauney vertex

 right now I cant get OUTEDGES!!
 can I iterate through and store for each vertex the 3 outedges, sure why not
 just in an array? do points have an index? maybe I could hash the points lol


 cant you just have

 array of faces
 array of vertices
 array of edges


Voronoi:

maybe i just calculate this from my library voronoi diagram lol. Rather than implement bowyer-watson myself

 F: (delauney V)
    - idx of N edges
    - site
 V: (delauney F)
    - idx of 3 edges
    - idx of 3 faces
    - site
 E:
    - idx of 2 verts
    - idx of 2 faces


Bowyer watson algorithm is a simple algorithm for obtaining delauney triangulation
can use it for voronoi presumably its simple enough to switch over