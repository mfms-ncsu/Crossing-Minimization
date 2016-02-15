/**
 * Implements a Graph ADT designed specifically for reading, translating, and
 * manipulating dags organized into layers for the purpose of solving
 * crossing minimization problems.
 *
 * Applications include:
 * - translating among different representations
 * - generating drawings and animations
 * - generating random instances
 * - (possibly) coding prototypes of algorithms
 *
 * A node is represented internally as an object with a name, adjacency
 * list, layer, and other information as appropriate.
 * Externally, a node is a string, i.e., its name.
 * Dummy nodes are given names of the form _d_n, where n is the node number.
 *
 * A layer is a (linked) list of nodes, sorted from 'left' to 'right'.
 *
 * The set of nodes, the adjacency list of a node, and each layer are
 * available in the form of iterators. 
 *
 * @author Matt Stallmann, 2008/09/25
 * @version 2008/09/25
 *
 * $Id: LayeredDag.java 19 2011-06-23 01:46:27Z mfms $
 */

import java.util.TreeMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.ArrayList;
import java.util.Set;

public class LayeredDag {

    private static final boolean DEBUG = false;

  private String graphName;
  private String [] comments;

  /** Maps a name (external representation) to an internal node */
  private TreeMap< String, Node > nodeList;

  private LayerList layerList;
  private int numberOfRegularNodes = 0;
  private int numberOfDummyNodes = 0;
  private int numberOfEdges = 0;

  /**
   * Creates a name of the form prefix_AAAA, where AAAA is a 4-digit
   * representation of the number padded with 0's
   */
  public static String numberToName( String prefix, int number )
  {
    return String.format( "%s_%d", prefix, number );
  }

  /**
   * A node of the graph.
   * @todo Now that the fields are public, their access methods are no longer
   * needed, but this would require some global changes.
   */
  private class Node {
    public String name;
    public int layer;

    /** adjacency list, i.e., nodes on a higher layer to which this
     * one is connected, or, the endpoints of the out edges */
    public LinkedList< Node > adjacencyList;
    /** 
     *current number of incoming edges; useful during topological sort,
     * and therefore not guaranteed to be consistent
     * @todo Need to maintain a permanent in degree that can only be
     * incremented and accessed
     */
    public int inDegree;

    public Node( String name )
    {
      this.name = name;
      adjacencyList = new LinkedList< Node >();
      layer = -1;               // an illegal layer so that we can check
                                // whether a node already has a layer
      inDegree = 0;
    }

    public String getName() { return name; }

    public boolean hasLayer() { return layer != -1; }

    public int getLayer() { return layer; }

    public int getInDegree() { return inDegree; }

    public LinkedList< Node > getAdjacencyList()
    {
      return adjacencyList;
    }

    public void addEdge( Node otherNode )
    {
      adjacencyList.addLast( otherNode );
    }

    public void deleteEdge( Node otherNode )
    {
      adjacencyList.remove( otherNode );
    }

    public void setLayer( int layer ) { this.layer = layer; }

    public void incInDegree() { inDegree++; }

    public void decInDegree() { inDegree--; }
    
    public void clearInDegree() { inDegree = 0; }

    public String toString() { return name + "," + layer; }
  } // end inner Node class

  /**
   * Maintains a layer, a list of strings representing the nodes of the
   * layer.
   */
  class Layer {
    public ArrayList< String > nodes;

    public Layer()
    {
      nodes = new ArrayList< String >();
    }

    public void add( String node )
    {
      nodes.add( node );
    }

    public void remove( String node )
    {
      nodes.remove( node );
    }

    /**
     * @return index of the position of the node on the layer or -1 if it
     * does not exist
     */
    public int getPosition( String node )
    {
      return nodes.indexOf( node );
    }

    public Iterator< String > getNodes()
    {
      return nodes.iterator();
    }
  } // end inner Layer class
  
  /**
   * Maintains a list of layers, each layer being a list of strings, the
   * names of the nodes on that layer.
   */
  class LayerList {
    public ArrayList< Layer > layers;

    public LayerList()
    {
      layers = new ArrayList< Layer >();
    }

    public void add( int layerNumber, String node )
    {
      if( layerNumber >= layers.size() )
        { // need to add new layer(s)
          int oldSize = layers.size();
          for( int i = oldSize; i <= layerNumber; i++ )
            {
              layers.add( new Layer() );
            }
        }
      layers.get( layerNumber ).add( node );
    }

    public void remove( int layerNumber, String node )
    {
      layers.get( layerNumber ).remove( node );
    }

    public int size() { return layers.size(); }

    public Iterator< String > getNodes( int layerNumber )
    {
      return layers.get( layerNumber ).getNodes();
    }
  } // end inner LayerList class

  public LayeredDag()
  {
    nodeList = new TreeMap< String, Node >();
    layerList = new LayerList();
    comments = new String[0];
  }

  public String getGraphName() { return graphName; }

  public void setGraphName( String name ) { graphName = name; }

  public String [] getComments() { return comments; }

  public void addComment( String comment )
  {
    String [] temp = new String[ comments.length + 1 ];
    int i;
    for ( i = 0; i < comments.length; i++ ) temp[i] = comments[i];
    //    temp[i] = new String( comment );
    temp[i] = comment;
    comments = temp;
  }

  public boolean nodeNotPresent( String name )
  {
    return ! nodeList.containsKey( name );
  }

  private Node getNodeFromName( String name )
  {
    return nodeList.get( name );
  }

  public int getNumberOfNodes()
  {
    return numberOfRegularNodes + numberOfDummyNodes;
  }

  public int getNumberOfRegularNodes()
  {
    return numberOfRegularNodes;
  }

  public int getNumberOfDummyNodes()
  {
    return numberOfDummyNodes;
  }

  public int getNumberOfEdges()
  {
    return numberOfEdges;
  }

  public int getNumberOfLayers()
  {
    return layerList.size();
  }

  public int getLayerSize( int layerNumber )
  {
    return layerList.layers.get( layerNumber ).nodes.size();
  }

  public int getLayer( String nodeName )
  {
    if( nodeNotPresent( nodeName ) )
      {
        System.out.printf("getLayer: Unable to find node %s\n", nodeName);
        return 0;
      }
    return getNodeFromName(nodeName).layer;
  }

  public int getPositionInLayer( String nodeName )
  {
    Node node = getNodeFromName( nodeName );
    Layer layer = layerList.layers.get( node.getLayer() );
    return layer.getPosition( nodeName );
  }

  public void setLayer( String nodeName, int layer )
  { 
    if( nodeNotPresent( nodeName ) )
      {
        System.out.printf("setLayer(%d): Unable to find node %s\n",
                          layer, nodeName);
        return;
      }
    Node node = getNodeFromName( nodeName );
    if( node.getLayer() != -1 )
      layerList.remove( node.getLayer(), nodeName );
    getNodeFromName(nodeName).setLayer( layer );
    layerList.add( layer, nodeName );
  }

  public void incInDegree( String nodeName )
  {
    getNodeFromName(nodeName).incInDegree();
  }

  public void decInDegree( String nodeName )
  {
    getNodeFromName(nodeName).decInDegree();
  }

  public void clearInDegree( String nodeName )
  {
    getNodeFromName(nodeName).clearInDegree();
  }

  public int getInDegree( String nodeName ) {
    return getNodeFromName(nodeName).getInDegree();
  }

  /**
   * @return true if the node is already in the list
   */
  public boolean isANode( String name )
  {
    return nodeList.containsKey( name );
  }

    /**
     * Adds a regular - as opposed to dummy - node; assumes that the node does
     * not exist yet
     */
    public void addNode( String name )
    {
        if ( DEBUG )
            System.out.printf( "-> addNode %s\n", name );
        nodeList.put( name, new Node( name) );
        numberOfRegularNodes++;
    }

  /**
   * Used to add dummy nodes as needed.
   * @return the name of a new node that is not included in the original dag,
   * of the form _d_x, where x is the current number of dummy nodes
   */
  public String addDummyNode()
  {
    String newName = numberToName( "_d", numberOfDummyNodes );
    Node newNode = new Node( newName );
    nodeList.put( newName, newNode );
    numberOfDummyNodes++;
    return newName;
  }

  /**
   * Adds an edge: nodes are given by their names;
   */
  public void addEdge( String fromName, String toName )
  {
    if( nodeNotPresent( fromName ) )
      {
        System.out.printf("WARNING: addEdge, missing node '%s'\n"
                          + "Edge %s -> %s ignored.\n",
                          fromName, fromName, toName );
        return;
      }
    if( nodeNotPresent( toName ) )
      {
        System.out.printf("WARNING: addEdge, missing node '%s'\n"
                          + "Edge %s -> %s ignored.\n",
                          toName, fromName, toName );
        return;
      }
    getNodeFromName( fromName ).addEdge( getNodeFromName( toName ) );
    incInDegree( toName );
    numberOfEdges++;
  }

  public void deleteEdge( String fromName, String toName )
  {
    getNodeFromName( fromName ).deleteEdge( getNodeFromName( toName ) );
    numberOfEdges--;
  }

  /**
   * @param nodeList A list of Nodes
   * @return A linked list containing the names of the nodes in nodeList
   * @todo There's probably an easier way to do this via some kind of mapping
   * abstraction.
   */
  private LinkedList< String > nodesToNames( LinkedList<Node> nodes )
  {
    LinkedList< String > nameList = new LinkedList< String >();
    for( Node node: nodes )
      {
        nameList.addLast( node.getName() );
      }
    return nameList;
  }

  /**
   * @return an iterator over the adjacency list of a node, specified by its
   * name; nodes are also specified by name
   */
  public Iterator< String > adjacencyList( String name )
  {
    return nodesToNames( getNodeFromName( name ).getAdjacencyList() )
      .listIterator();
  } 

  /**
   * @return an array containing the neighbors of a node, specified by their
   * names; this is useful when the dummy nodes are being added during a scan.
   */
  public String [] adjacencyListArray( String name )
  {
    String [] nameArray = new String[0];
    LinkedList<String> nameList
      = nodesToNames( getNodeFromName( name ).getAdjacencyList() );
    return nameList.< String > toArray( nameArray );
  } 

  /**
   * @return an iterator over the nodes of the graph, each item being the
   * name of a node
   */
  public Iterator< String > nodes()
  {
    return nodeList.keySet().iterator();
  }

  /**
   * @return an array containing the nodes of the graph, each item being the
   * name of a node; this is needed if dummy nodes are being added as we
   * traverse the regular nodes and their adjacency lists. 
   * @todo Not clear why the cast is needed, but compiler complains without it
   */
  public String [] nodeArray()
  {
    String [] nameArray = new String[0];
    Set<String> nameSet
      = nodeList.keySet();
    return nameSet.< String > toArray( nameArray );
  }

  /**
   * Iterates over the nodes on a layer.
   */
  public Iterator< String > nodesOnLayer( int layer )
  {
    return layerList.getNodes( layer );
  }

  public String [] nodesOnLayerArray( int layerNumber )
  {
    String [] nodeArray = new String[0];
    Layer layer = layerList.layers.get( layerNumber );
    ArrayList< String > nodes = layer.nodes;
    return nodes.< String > toArray( nodeArray );
  }

    public String toString()
    {
        String string = "-> LayeredDag ->";
        string += " number of layers = " + getNumberOfLayers() + "\n";
        for( int i = 0; i < getNumberOfLayers(); i++ )
            {
                // add each layer
                string += "  layer " + i + ":";
                Iterator<String> nodes = nodesOnLayer( i );
                while( nodes.hasNext() )
                    {
                        string += " " + nodes.next();
                    }
                string += "\n";
            }
        String [] nodeArray = nodeArray();
        for( int i = 0; i < nodeArray.length; i++ )
            {
                String nextName = nodeArray[i];
                string += "    node " + nextName + ":";
                String [] theNeighbors
                    = adjacencyListArray( nextName );
                for( int j = 0; j < theNeighbors.length; j++ )
                    string += " " + theNeighbors[j];
                string += "\n";
            }
        string += "<- END LayeredDag";
        return string;
    }

  /**
   * a simple test program
   */
    public static void main(String [] args)
    {
        LayeredDag sg = new LayeredDag();
        for( int i = 0; i < 11; i++ ) {
            sg.addNode( "" + i );
            sg.setLayer( "" + i, i / 3 );
        }
        for( int i = 0; i < 7; i++ )
            {
                sg.addEdge( "" + i, "" + ((i + 2) % 11) );
                sg.addEdge( "" + i, "" + ((i + 3) % 11) );
            }
        for( int i = 7; i < 10; i++ ) sg.addEdge( "" + i, "" + ((i + 5) % 11) );
        String dn = sg.addDummyNode();
        sg.addEdge( dn, "" + 5 );
        sg.setLayer( dn, 5 );
        String dntwo = sg.addDummyNode();
        sg.setLayer( dntwo, 1 );

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
        System.out.println( "" + sg);
  } // end, main
} // end, class

//  [Last modified: 2011 06 16 at 18:59:31 GMT]
