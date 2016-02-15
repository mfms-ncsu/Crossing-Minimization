/**
 * Reads the .dot and .ord files for a dag and stores the dag, making it
 * available to other programs. Assumes a simplified .dot format:
 *
 * <pre>
 * comments (assumes each comment has one token before the text of the comment)
 * digraph NAME {
 *   v_1 -> w_1;
 *   ...
 *   v_m -> w_m; 
 * }
 * </pre>
 *
 * For the reader to work correctly, there must be white space between each
 * node name and the ->, and the second node name must be followed
 * immediately by the semicolon (no white space)
 *
 * @author Matt Stallmann, 2008/10/06
 * @version 2008/10/06
 * $Id: DagReader.java 59 2014-03-15 16:49:13Z mfms $
 */

import java.util.Scanner;
import java.io.*;
import java.util.Iterator;      // for testing in main program

public class DagReader {

    private static final boolean DEBUG = false;

  private LayeredDag graph;
  private Scanner dotInput;
  private Scanner ordInput;
  /** true if layering will be specified by an ord file */
  private boolean layered;
    // private int numberOfIsolatedNodes;

  /** scanner attached to the line of a file that is currently being
   * processed */
  private Scanner currentLine;

  /**
   * Creates a new graph
   * @param dotInput scanner for reading dot file
   * @param ordInput scanner for reading ord file
   */
  public DagReader( Scanner dotInput, Scanner ordInput )
  {
    graph = new LayeredDag();
    this.dotInput = dotInput;
    this.ordInput = ordInput;
    // numberOfIsolatedNodes = 0;
    layered = true;
  }

  /**
   * Creates a new graph without layering or ordering information; all layers
   * are -1 and order is based on appearance in the dot file.
   * @param dotInput scanner for reading dot file
   */
  public DagReader( Scanner dotInput )
  {
    graph = new LayeredDag();
    this.dotInput = dotInput;
    // numberOfIsolatedNodes = 0;
    layered = false;
  }

//   /**
//    * Returns the number of isolated nodes discovered while reading; these are
//    * nodes that show up in the .ord file but not in the .dot file
//    */
//   public int getNumberOfIsolatedNodes()
//   {
//     return numberOfIsolatedNodes;
//   }

  /**
   * Reads the graph from the scanners dotInput and ordInput
   */
  public void readGraph()
  {
    if ( layered ) readOrd();
    readDot();
  }

  /**
   * Returns the graph; presumably after it has been read in
   */
  public LayeredDag getGraph() { return graph; }

  /**
   * Reads the dot file from the dotInput scanner
   */
  private void readDot()
  {
    readDotPreamble();
    while( readEdge() );
  }

  /**
   * Reads comments and graph name
   */
  private void readDotPreamble()
  {
    while ( dotInput.hasNextLine() )
      {
        String line = dotInput.nextLine();
        currentLine = new Scanner( line );
        String first = "";
        if ( currentLine.hasNext() )
            first = currentLine.next();
        if ( first.equals( "digraph" ) )
          break;
        if ( first.equals( "/*" )
             || first.equals( "*/" ) )
          continue;
        String comment = "";
        while ( currentLine.hasNext() )
          comment += " " + currentLine.next();
        graph.addComment( comment );
      }
    if ( ! currentLine.hasNext() )
      {
        System.out.println( "*** Error: missing graph name in dot file." );
      }
    graph.setGraphName( currentLine.next() );
  }

  /**
   * Reads the name of the graph. Assumes the comments and the word digraph
   * have been read and the latter is on the current line.
   */

  /**
   * Reads the next edge from the dot file
   * @return true if another edge exists
   */
  private boolean readEdge()
  {
    if( ! dotInput.hasNext() )
      {
        System.out.printf("Oops: missing } at end of file.\n");
        return false;
      }
    String sourceName = dotInput.next();
    if( sourceName.equals("}") ) return false;
    if( ! graph.isANode( sourceName ) ) graph.addNode( sourceName );
    String arrow = dotInput.next();
    if( ! arrow.equals("->") )
      {
        System.out.printf("Oops: %s instead of ->\n", arrow);
        return false;
      }
    String destinationName = dotInput.next();
    if( ! destinationName.endsWith(";") )
      {
        System.out.printf("Oops: %s should end with ;\n", destinationName);
        return false;
      }
    // strip off the trailing ;
    destinationName
      = destinationName.substring( 0, destinationName.length() - 1 );
    if( ! graph.isANode( destinationName ) ) graph.addNode( destinationName );
    graph.addEdge( sourceName, destinationName );
    return true;
  }

  private enum State { NOT_IN_LAYER, LAYER_NUMBER, IN_LAYER }

  /**
   * Reads an ord file assigning layers to the nodes
   */
  private void readOrd()
  {
    currentLine = new Scanner("");
    State state = State.NOT_IN_LAYER;
    int currentLayer = -1;
    while( hasNextToken() )
      {
        String token = nextToken();
        switch( state ) {
        case NOT_IN_LAYER:
          currentLayer = Integer.parseInt( token );
          state = State.LAYER_NUMBER;
          break;
        case LAYER_NUMBER:
          if( ! token.equals("{") )
            {
              System.out.printf("Expecting { in ord file, got %s instead\n",
                                token);
              return;
            }
          state = State.IN_LAYER;
          break;
        case IN_LAYER:
          if( token.equals("}") ) state = State.NOT_IN_LAYER;
          else
            {
              if( graph.nodeNotPresent( token ) )
                {
                    graph.addNode( token );
                }
              graph.setLayer( token, currentLayer );
            }
          break;
        }
      }
  }

  /** 
   * Removes everything starting with the first # of the string.
   * @return the string with any comment (stuff starting with #) removed
   */
  private static String stripComment( String line )
  {
    int commentPosition = line.indexOf('#');
    if( commentPosition == -1 ) return line;
    return line.substring( 0, commentPosition );
  }

  /**
   * @return true if there is still something other than a comment in the
   * input.
   */
  private boolean hasNextToken()
  {
    if( currentLine.hasNext() ) return true;
    while( ordInput.hasNextLine() )
      {
        String nextLine = ordInput.nextLine();
        nextLine = stripComment( nextLine );
        currentLine = new Scanner( nextLine );
        if( currentLine.hasNext() ) return true;
      }
    return false;
  }

  /**
   * @return the next input token in the ord file, ignoring comments  
   * Assumes that hasNextToken() is true, i.e., the current line has a next
   * item.
   */
  private String nextToken()
  {
    return currentLine.next();
  }

  public static void main(String [] args)
  {
    String baseName = args[0];
    String dotFileName = baseName + ".dot";
    String ordFileName = baseName + ".ord";
    Scanner dotReader = null;
    Scanner ordReader = null;
    try
      {
         dotReader = new Scanner( new FileReader( dotFileName ) );
      }
    catch (IOException exception)
      {
        System.out.println("Unable to read file " + dotFileName );
      }
    try
      {
         ordReader = new Scanner( new FileReader( ordFileName ) );
      }
    catch (IOException exception)
      {
        System.out.println("Unable to read file " + ordFileName );
      }

    DagReader reader = new DagReader( dotReader, ordReader );
    reader.readGraph();
    LayeredDag sg = reader.getGraph();

    Iterator< String > theNodes = sg.nodes();
    while( theNodes.hasNext() )
      {
        String nextName = theNodes.next();
        System.out.println("Node " + nextName);
        Iterator< String > theNeighbors
          = sg.adjacencyList( nextName );
        while( theNeighbors.hasNext() )
          System.out.println(" " + theNeighbors.next() );
      }
    System.out.println("Number of layers = " + sg.getNumberOfLayers());
    for( int i = 0; i < sg.getNumberOfLayers(); i++ )
      {
        // print each layer
        System.out.println("Layer " + i + ":");
        Iterator<String> nodes = sg.nodesOnLayer( i );
        while( nodes.hasNext() )
          {
            System.out.println(" " + nodes.next() );
          }
      }

    dotReader.close();
    ordReader.close();
  } // end, main
} // end, class

//  [Last modified: 2014 01 28 at 20:14:00 GMT]
