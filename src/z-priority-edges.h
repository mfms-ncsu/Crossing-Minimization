/**
 * @author Matt Stallmann
  $Id: z-priority-edges.h 2 2011-06-07 19:50:41Z mfms $
 */

/**
   Some thoughts on how to modify heuristics so that specific edges, e.g.,
   those that are predecessors or successors of a given node as in the ABM
   application, are given priority when it comes to minimizing crossings.

   - Experiments can initially be done using the middle node of the middle
     layer. If input order has been scrambled this is sutiably random and
     does not require an additional input file
     
   - The simplest approach initially is to keep track of the edge with the
     maximum number of crossings among the priority edges. A mechanism
     already exists for finding the maximum crossings edge overall. It would
     then be easy to identify the iteration at which this number is
     minimized.

   - Total crossings among priority edges may be harder to track. But maybe
     the same mechanism can be used. Both the total and the max edge
     crossings are updated incrementally for all the heuristics.

  - It appears that an edge struct already has a field for the number of
    crossings. That makes life a lot easier.
 
  - The final challenge is to modify heurisitics so that the priority edges
     are favored in some way.
 */

/*  [Last modified: 2011 04 04 at 14:27:21 GMT] */
