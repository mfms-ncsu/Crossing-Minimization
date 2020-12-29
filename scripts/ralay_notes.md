[//]: # Detailed description of how ralay.py works.

Some notes on generation of a random layered graph using ralay.py (for **ra**ndom **lay**ered graph).
========================================
**Command line**
User specifies number of nodes, edges, and layers
_Other options:_
* a random seed (-s, --seed)
* degree variance (-dv, --deg_variance); see below
* minimum layer width, i.e., nodes per layer (-mw, --min_width)
* maximum layer width (-MW, --max_width)
* shape (--shape); details later
* number of dummy nodes (--dummies)

**Degree variance** is the most complex parameter to control. Whenever a neighbor w needs to be chosen for a node v, the following restrictions apply:
* if v is not on the lowest layer and has no incoming edges, w must come from a lower layer than v
* if v is not on the highest layer and has no outgoing edges, w must come from a higher layer
* otherwise w is unrestricted, except that it must come from a neighboring layer if there are no dummy nodes (the choice is more complicated when dummy nodes are possible - see below)

Certain potential neighbors have priority and the selection will be restricted to them if any exist
* a w on a lower layer with no outgoing edges
* a w on a higher layer with no incoming edges

Now for how degree variance works. Let N be the list of potential neighbors based on the above criteria. Divide N into buckets N[1], ... , N[max], where each N[d] contains nodes with indegree or outdegree d, as appropriate (indegree for nodes on higher layers, outdegree for nodes on lower layers); here, max_k is the maximum degree among the the potential neighbors.
Let M[k] = the k-th nonempty bucket, for k = 0, ... , max_k-1
* If deg_variance (dv) is 0, chose a random node w from M[0]
* If 0 < dv <= 1, let r be a random number in the interval [0,dv) and choose a random w from M[floor(r*max_k)]
* If dv > 1, let r be a random number from  an exponential distribution with mean 1/dv and choose w randomly from M[k], where k = floor(max_k * (1 - r)) and let k = 0 if it's negative. The goal is to get a small number of high degree nodes tailing off to larger numbers of low degree nodes. Intuition is that, by preferring high degree nodes when adding edges, the degrees of the lower degree nodes are not likely to increase.

**Layer width** is controlled by the ``-mw`` and ``-MW`` options, referred to as *m* and *M* in what follows. The number of nodes on each layer is chosen uniformly at random from the interval [*m*,*M*] with the following restrictions, letting *u* be the number of layers whose width will be chosen after that of the current layer and *n'* be the number of nodes not yet placed on layers (including those to be placed on the current layer).
* the invariant *n'* >= *u m* must be preserved, otherwise there are not enough nodes to fill the remaining layers
* the invariant *n'* <= *u M* must be preserved, otherwise the remaining nodes can't fit on the remaining layers
The restricted interval is therefore [*n'* ``-`` *u M*, *n'* ``-`` *u m*].
In summary, the lower bound is max(*m, n'-uM*) and the upper bound is min(*M, n'-um*)

**Shape** is determined by the *layer profile*, the list of layer widths. If shape is ``random``, the profile is determined by the random order in which layer widths were generated. If shape is anything other than ``random`` (the default), let *L* be the list of layer widths, sorted in ascending order. The options are:
* ``sorted`` - use *L* directly
* ``balloon`` -  largest width in the middle and decrease size moving toward the ends, alternating sides
* ``hourglass`` - the inverse of ``balloon``, smallest width in the middle
* ``sawtooth`` - "unshuffle" *L* into two equal-sized sorted lists and concatenate them
*``alternating`` - alternate small and large layers

**Dummy nodes** can be added to each edge, the number in the range [0,|*L*|-2] (upper end is max number of dummies when endpoints of the original edge are on top and bottom layers). Further restriction is determined by the ``--dummies`` parameter. Let *d* be the number of dummy nodes yet to be assigned, initially the ``--dummies`` parameter and let *m'* be the number of edges remaining *after* the one to which we are about to add dummy nodes. Then, in fashion similar to the restrictions on layer widths,
* we need *m'* (|*L*|-2]) >= *d*, otherwise there are not enough edges left to accomodate the remaining dummies
* obviously, we can't choose more than *d* dummies for the current edge
So the lower bound is max(0,*d-m'D*) and the upper bound is min(*d,D*), where *D* is |*L*|-2.

There is a catch, however: if both endpoints of an edge are not on outer layers, we cannot insert |*L*|-2 dummy nodes. The only guarantee is that we can always insert at least floor((|*L*|-2) / 2). So we need to let *D* = floor((|*L*|-2) / 2) in the above bounds.

**Note:** It may be better to have node *v* pick a neighbor from the set of layers that will satisfy the restrictions on dummy nodes, rather than picking a number of dummy nodes and then a neighbor for *v* on an appropriate layer.

When there are dummy nodes, the restriction that edges connect nodes on adjacent layers is modified appropriately
