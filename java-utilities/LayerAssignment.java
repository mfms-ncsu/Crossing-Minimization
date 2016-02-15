/**
 * Handles layer assignment for an arbitrary directed acyclic graph using
 * topological sort and inserting dummy nodes as necessary.  The input graph
 * is assumed to be in a simplified .dot format as described in
 * DagReader.java. Also uses LayeredDag.java to store the result. 
 *
 * @author Matt Stallmann 2009/04/20
 * $Id: LayerAssignment.java 78 2014-07-23 20:35:44Z mfms $
 */

import java.util.Scanner;
import java.io.*;
import java.util.Queue;
import java.util.LinkedList;
import java.util.Iterator;

public class LayerAssignment {

    private static final boolean DEBUG = false;

  private static void printUsage()
  {
    System.out.println("Usage: java LayerAssignment input.dot basename");
    System.out.println("   where basename is used to create names for the"
                       + " output .dot and .ord files");
  }

  /** Used when printing the .ord file */
  private static final int MAX_NODES_PER_LINE = 8;

  private LayeredDag graph;

  public LayerAssignment()
  {
    graph = new LayeredDag();
  }

  public LayeredDag getGraph() { return graph; }

  /**
   * Reads a dot file using the DagReader class
   */
  public void readInput( Scanner inputReader )
  {
    DagReader reader = new DagReader( inputReader );
    reader.readGraph();
    graph = reader.getGraph();
  }

  public void writeDot( PrintWriter dot )
  {
    String [] comments = graph.getComments();
    dot.println( "/*" );
    for ( int i = 0; i < comments.length; i++ )
      dot.printf( " * %s\n", comments[i] );
    dot.println( " */" );
    dot.printf( "digraph %s {\n", graph.getGraphName() );
    Iterator< String > nodes = graph.nodes();
    while( nodes.hasNext() )
      {
        String node = nodes.next();
        Iterator< String > neighbors = graph.adjacencyList( node );
        while( neighbors.hasNext() )
          {
            String neighbor = neighbors.next();
            dot.printf( " %s -> %s;\n", node, neighbor );
          }
      }
    dot.printf( "}\n" );
  }

  private void writeLayer( PrintWriter ord, int layer )
  {
    ord.printf( "\n# Ordering for layer %d\n", layer );
    ord.printf( "%d {\n", layer );
    Iterator< String > nodes = graph.nodesOnLayer( layer );
    int numberPrinted = 0;
    while( nodes.hasNext() )
      {
        ord.printf( " %s", nodes.next() );
        numberPrinted++;
        if( numberPrinted % MAX_NODES_PER_LINE == 0 )
          ord.println();
      }
    ord.printf( "\n} # end of layer %d\n", layer );
  }

  public void writeOrd( PrintWriter ord, String baseName )
  {
    ord.printf( "# LayerAssignment.java for %s.dot\n", baseName );
    ord.printf( "# Graph name = %s\n", graph.getGraphName() );
    for( int i = 0; i < graph.getNumberOfLayers(); i++ )
      writeLayer( ord, i );
  }

  /**
   * a queue of nodes using their string representation
   * Invariant: indegree of a node == 0 iff the node is on the queue
   */
  private Queue<String> queue;

  /**
   * Initializes the queue by putting all in-degree 0 nodes on it and putting
   * them on layer 0; the queue is implemented as a linked list
   */
  private void initQueue()
  {
    queue = new LinkedList<String>();
    Iterator< String > nodes = graph.nodes();
    while( nodes.hasNext() )
      {
        String node = nodes.next();
        if( graph.getInDegree( node ) == 0 )
          {
            graph.setLayer( node, 0 );
            queue.offer( node );
          }
      }
    if( queue.peek() == null )
      {
        System.out.printf("Graph has no source, cannot proceed\n");
        System.exit(1);
      }
  }

  /**
   * Puts a node in the queue.
   */
  private void enqueue( String node )
  {
    queue.offer( node );
  }

 /**
  * @return The front node on the queue (and remove it).
  */
  private String dequeue()
  {
    String node = queue.remove();
    return node;
  }

  /**
   * Updates information about the outgoing neighbors of a node, i.e.,
   * decreases their indegree and puts them on the queue if it is 0.
   */
  private void processNode( String node )
  {
      if ( DEBUG )
          System.out.printf( "-> processNode: %s\n", node );
    Iterator< String > neighbors = graph.adjacencyList( node );
    while( neighbors.hasNext() )
      {
        String neighbor = neighbors.next();
        if ( DEBUG )
                System.out.printf( "  processNode: %s -> %s\n",
                                   node, neighbor );
        if ( graph.getLayer( neighbor ) != -1 )
          {
            System.out.printf( "Target already has layer when processing"
                               + " '%s -> %s\n", node, neighbor );
          }
        graph.decInDegree( neighbor );
        if ( DEBUG )
                System.out.printf( "    processNode: indeg(%s) = %d\n",
                                   neighbor, graph.getInDegree( neighbor ) );
        if ( graph.getInDegree( neighbor ) == 0 )
          {
            graph.setLayer( neighbor, graph.getLayer( node ) + 1 );
            enqueue( neighbor );
          }
      } // end for all neighbors of node
  }

  /**
   * Inserts dummy node(s) between two given nodes
   */
  private void insertDummyNodes( String src, String dest )
  {
    int srcLayer = graph.getLayer( src );
    int destLayer = graph.getLayer( dest );
    if( srcLayer >= destLayer )
      {
        System.out.printf("insertDummyNodes: Bad layers\n"
                          + " %s -> %s, %d -> %d\n",
                          src, dest, srcLayer, destLayer);
        System.exit(1);
      }
    String previousNode = src;
    for( int i = srcLayer + 1;
         i < destLayer;
         i++ )
      {
        String newNode = graph.addDummyNode();
        graph.setLayer( newNode, i );
        graph.addEdge( previousNode, newNode );
        if ( DEBUG )
            System.out.printf(" Adding edge with dummy node %s -> %s\n",
                              previousNode, newNode);
        previousNode = newNode;
      }
    graph.addEdge( previousNode, dest );
    if ( DEBUG )
        System.out.printf(" Adding edge with dummy node %s -> %s\n",
                      previousNode, dest);
    graph.deleteEdge( src, dest );
  }

  /**
   * Inserts dummy nodes and corresponding edges as needed when an edge does
   * not go to the next layer.
   */
  private void insertAllDummyNodes()
  {
    // go through all the edges to check for necessary insertions
    String [] nodes = graph.nodeArray();
    for( int i = 0; i < nodes.length; i++ )
      {
        String node = nodes[i];
        String [] neighbors = graph.adjacencyListArray( node );
        for( int j = 0; j < neighbors.length; j++ )
          {
            String neighbor = neighbors[j];
            int srcLayer = graph.getLayer( node );
            int destLayer = graph.getLayer( neighbor );
            if( destLayer - srcLayer > 1 )
              insertDummyNodes( node, neighbor );
          }
      }
  }

  /**
   * Does a topological sort to assign layers.
   */
  private void assignLayers()
  {
    initQueue();
    while( queue.peek() != null )
      {
        String node = dequeue();
        processNode( node );
      }
  }

    /**
     * Adds new comments about added dummy nodes and number of edges after
     * layering.
     */
    private void addComments() {
        graph.addComment( "Layer assignment done: "
                          + graph.getNumberOfDummyNodes()
                          + " dummy nodes added." );
        graph.addComment( "total nodes = "
                          + graph.getNumberOfNodes()
                          + ", edges = "
                          + graph.getNumberOfEdges()
                          + ", layers = "
                          + graph.getNumberOfLayers()
                          );
    }

  /**
   * Handles commandline arguments, opens files, and initiates processing
   */
  public static void main(String [] args)
  {
    if( args.length != 2 )
      {
        printUsage();
        return;
      }
    String inputFileName = args[0];
    String baseName = args[1];
    String dotFileName = baseName + ".dot";
    String ordFileName = baseName + ".ord";
    Scanner inputReader = null;
    PrintWriter dotWriter = null;
    PrintWriter ordWriter = null;
    try
      {
         inputReader = new Scanner( new FileReader( inputFileName ) );
      }
    catch (IOException exception)
      {
        System.out.println("Unable to read file " + inputFileName );
      }

    LayerAssignment assigner = new LayerAssignment();

    assigner.readInput( inputReader ); 
    inputReader.close();

    assigner.assignLayers();
    assigner.insertAllDummyNodes();
    assigner.addComments();

    try
      {
        dotWriter = new PrintWriter( dotFileName );
      }
    catch (IOException exception)
      {
        System.out.println("Unable to write to file " + dotFileName );
      }
    try
      {
        ordWriter = new PrintWriter( ordFileName );
      }
    catch (IOException exception)
      {
        System.out.println("Unable to write to file " + ordFileName );
      }

    assigner.writeDot( dotWriter );
    assigner.writeOrd( ordWriter, baseName );

    System.err.printf( "original nodes  = %8d\n",
                       assigner.getGraph().getNumberOfRegularNodes() );
    System.err.printf( "dummy nodes     = %8d\n",
                       assigner.getGraph().getNumberOfDummyNodes() );
    System.err.printf( "edges           = %8d\n",
                        assigner.getGraph().getNumberOfEdges() );
    System.err.printf( "layers          = %3d\n",
                        assigner.getGraph().getNumberOfLayers() );

    dotWriter.close();
    ordWriter.close();
  } // end, main
} // end, class

//  [Last modified: 2014 07 23 at 20:08:48 GMT]
