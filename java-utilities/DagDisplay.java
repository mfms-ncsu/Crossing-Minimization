/**
 * Displays a layered dag given by a .dot file using the ordering specified
 * in a given .ord file
 *
 * @author Matt Stallmann, 2008/10/08
 * @version 2008/10/08
 *
 * $Id: DagDisplay.java 59 2014-03-15 16:49:13Z mfms $
 */

import java.util.Scanner;
import java.io.*;
import java.awt.*;
import java.awt.geom.*;
import javax.swing.*;
import java.util.Iterator;

public class DagDisplay {
    static final boolean DEBUG = false;

    private static void printUsage()
    {
        System.out.println("Usage: java DagDisplay dotfile ordfile favored-edges"
                           + " width height"
                           + " label-size");
        System.out.printf( "where\n"
                           + "  dotfile is the graph in .dot format\n"
                           + "  ordfile gives the ordering of nodes on each layer in .ord format\n"
                           + "  favored-edges is a subgraph whose edges are to be highlighted, in .dot format [or '-' if none]\n"
                           + "  width and height are the width/height of the display window\n"
                           + "  label-size is the font size of labels, 0 if none are desired\n"
                           );
    }

    static final int MARGIN = 50;
    static final int NODE_SIZE = 7;
    static final int REGULAR_EDGE_THICKNESS = 2;
    static final int FAVORED_EDGE_THICKNESS = 4;
    static final Color REGULAR_COLOR = Color.BLACK;
    static final Color FAVORED_COLOR = Color.RED;
    /** use default font */
    static final String FONT = null;
    static final int FONT_STYLE = Font.PLAIN;
    
    static int fontSize = 10;
    static boolean noLabels = false;

    private int width;
    private int height;
    private LayeredDag graph;
    private LayeredDag favoredEdges;

    private Scanner dotReader;
    private Scanner ordReader;
    private Scanner favoredEdgeReader;

    private JFrame frame;
    private Canvas canvas;

    private void printStats( LayeredDag graph, DagReader graphReader )
    {
        double edgeRatio
            = (double) graph.getNumberOfEdges()
            / graph.getNumberOfNodes();
        System.out.printf( " nodes = %d, edges = %d,"
                           + " layers = %d, edge ratio = %5.2f\n",
                           graph.getNumberOfNodes(),
                           graph.getNumberOfEdges(),
                           graph.getNumberOfLayers(),
                           edgeRatio
                           );
    }

    /**
     * Responsible only for setting up the main window.
     */
    public DagDisplay( String title, int width, int height )
    {
        this.width = width;
        this.height = height;

        frame = new JFrame( title );
        frame.setSize(width, height);
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setBackground(Color.WHITE);
    }

    /**
     * Sets up the canvas (which calculates distances between nodes). 
     * Precondition: graph must exist
     */
    private void addCanvas()
    {
        canvas = new Canvas( graph, width, height );

        frame.add(canvas);
        frame.setVisible(true);
    }

    /**
     * The canvas on which the drawing is done.
     */
    private class Canvas extends JComponent {
        private int width;
        private int height;
        private LayeredDag graph;
        private int numberOfLayers;
        /** vertical distance between layers */
        private int layerSeparation;
        /** horizontal distance between nodes on each layer */
        private int [] nodeSeparation;
    
        /**
         * @param graph This is needed so that distances between layers and
         * between nodes on each layer can be calculated.
         */
        public Canvas( LayeredDag graph, int width, int height )
        {
            this.graph = graph;
            this.width = width;
            this.height = height;
            this.setBackground( Color.white );

            numberOfLayers = graph.getNumberOfLayers();
            if( numberOfLayers > 1 )
                layerSeparation = (height - 2 * MARGIN) / (numberOfLayers - 1); 
            else layerSeparation = 0;

            nodeSeparation = new int[ numberOfLayers ];
            for( int k = 0; k < numberOfLayers; k++ )
                {
                    int layerSize = graph.getLayerSize( k );
                    if( layerSize > 1 )
                        nodeSeparation[k] = (width - 2 * MARGIN) / (layerSize + 1);
                    else nodeSeparation[k] = width / 2;
                }
        }

        /**
         * @todo These need to be adjusted so that they return the top left
         * with respect to NODE_SIZE
         */
        private int xCoordinate( int layer, int nodePosition )
        {
            return MARGIN + (nodePosition + 1) * nodeSeparation[layer];
        }

        private int yCoordinate( int layer )
        {
            return MARGIN + (numberOfLayers - layer - 1) * layerSeparation;
        }

        /**
         * Draws all the nodes on the given layer
         * If nolabel, then labels are omitted
         */
       private void drawLayer( Graphics2D g2d, int layerNumber )
        {
            if ( DEBUG )
                System.err.printf( "-> drawLayer %d\n", layerNumber );
            String [] layerArray = graph.nodesOnLayerArray( layerNumber );
            int numberOfNodes = layerArray.length;
            for( int i = 0; i < numberOfNodes; i++ )
                {
                    g2d.draw(
                             new Ellipse2D.Double(
                                                  xCoordinate( layerNumber, i ),
                                                  yCoordinate( layerNumber ),
                                                  NODE_SIZE, NODE_SIZE
                                                  )
                             );
                    if( ! noLabels )
                        {
                            // could use getStringBounds() for the new font
                            // to clear an area for the label; would work
                            // well if the layer is drawn after the egdes and
                            // the rectangle is centered
                            g2d.setFont( new Font( FONT, FONT_STYLE, fontSize ) );
                            String nodeName = layerArray[i];
                            // this is kludgy; problem is that the names in
                            // synthetic dags are strings of the form n_x,
                            // where x is an integer; it appears that dot
                            // format also allows numerical node names, which
                            // would solve this problem, but might require
                            // some modification of other code
                            int delimiterPosition = nodeName.lastIndexOf('_');
                            String nodeNumber
                                = nodeName.substring( delimiterPosition + 1,
                                                      nodeName.length() );
                            g2d.drawString( nodeNumber,
                                          xCoordinate( layerNumber, i ) + NODE_SIZE + 1,
                                          yCoordinate( layerNumber ) + NODE_SIZE + 1 );
                        }
                }
            if ( DEBUG )
                System.err.printf( "<- drawLayer %d\n", layerNumber );
        }

        /**
         * @param node One endpoint of the edge
         * @param neighbor The other endpoint
         * @param favored true if the edges of this subgraph are to be
         * highlighted in some way (e.g., thicker)
         */
        void drawOneEdge( LayeredDag graph, String node, String neighbor,
                          Graphics2D g2d, boolean favored )
        {
            int nodeLayer = graph.getLayer( node );
            int neighborLayer = graph.getLayer( neighbor );
            int nodePosition = graph.getPositionInLayer( node );
            int neighborPosition = graph.getPositionInLayer( neighbor );
            if ( DEBUG )
                System.err.printf( "  drawEdges: neighbor = %s\n"
                                   + "     nodeLayer = %d, neighborLayer = %d\n"
                                   + "     nodePosition = %d, neighborPosition = %d\n",
                                   neighbor, nodeLayer, neighborLayer,
                                   nodePosition, neighborPosition );
            // line thickness
            if ( favored )
                {
                    g2d.setColor( FAVORED_COLOR );
                    g2d.setStroke( new BasicStroke( FAVORED_EDGE_THICKNESS ) );
                }
            else
                {
                    g2d.setColor( REGULAR_COLOR );
                    g2d.setStroke( new BasicStroke( REGULAR_EDGE_THICKNESS ) );
                }
            g2d.draw(
                     new Line2D.Double(
                                       xCoordinate( nodeLayer, nodePosition ),
                                       yCoordinate( nodeLayer ),
                                       xCoordinate( neighborLayer, neighborPosition),
                                       yCoordinate( neighborLayer )
                                       )
                     );
        }

        /**
         * Draws the edges incident on the given node
         * @param favored true if the edges of this subgraph are to be
         * highlighted in some way (e.g., thicker)
         */
        void drawIncidentEdges( LayeredDag graph, String node,
                                Graphics2D g2d, boolean favored )
        {
            Iterator< String > neighbors = graph.adjacencyList( node );
            while( neighbors.hasNext() )
                {
                    String neighbor = neighbors.next();
                    drawOneEdge( graph, node, neighbor, g2d, favored );
                }
        }

        /**
         * Draws edges of the given graph
         * @param favored true if the edges of this subgraph are to be
         * highlighted in some way (e.g., thicker)
         */
        private void drawEdges( LayeredDag graph, Graphics2D g2d, boolean favored )
        {
            if ( DEBUG )
                System.err.printf( "-> drawEdges %b\n", favored );
            Iterator< String > nodes = graph.nodes();
            while( nodes.hasNext() )
                {
                    String node = nodes.next();
                    if ( DEBUG )
                        System.err.printf( " drawEdges: node = %s\n", node );
                    drawIncidentEdges( graph, node, g2d, favored );
                }
            if ( DEBUG )
                System.err.printf( "<- drawEdges %b\n", favored );
        }

        public void paintComponent( Graphics g )
        {
            Graphics2D g2d = (Graphics2D) g;
            for( int k = 0; k < graph.getNumberOfLayers(); k++ )
                {
                    drawLayer( g2d, k );
                }
            drawEdges( graph, g2d, false );
            if ( favoredEdges != null )
                drawEdges( favoredEdges, g2d, true );
        }
    }

    /**
     * @param dotReader A stream that "contains" a set of edges in .dot
     * format
     * @param ordReader A stream that gives ordering of nodes on each layer
     * using .ord format
     * @return The dag derived from dotReader and ordReader
     */
    private LayeredDag initDag( Scanner dotReader,
                                Scanner ordReader
                                )
    {
        DagReader graphReader = new DagReader( dotReader, ordReader );
        graphReader.readGraph();
        LayeredDag localGraph = graphReader.getGraph();
        if ( DEBUG )
            System.out.print( graph + "\n" );

        printStats( localGraph, graphReader );
        return localGraph;
    }
    
    /**
     * Opens files and calls on initDag to create the appropriate layered dags
     */
    private void readDags( String dotFileName,
                           String ordFileName,
                           String favoredEdgeFileName )
    {
        try
            {
                dotReader = new Scanner( new FileReader( dotFileName ) );
            }
        catch (IOException exception)
            {
                System.out.println("Unable to read file " + dotFileName );
                System.exit(1);
            }
        try
            {
                ordReader = new Scanner( new FileReader( ordFileName ) );
            }
        catch (IOException exception)
            {
                System.out.println("Unable to read file " + ordFileName );
                System.exit(1);
            }

        System.out.println( "Main dag:" );
        graph = initDag( dotReader, ordReader );
        dotReader.close();
        ordReader.close();

        // need to open ordReader a second time if there are favored edges!
        if ( ! favoredEdgeFileName.equals("-") ) {
            try
                {
                    ordReader = new Scanner( new FileReader( ordFileName ) );
                }
            catch (IOException exception)
                {
                    System.out.println("Unable to read ord file second time, name = "
                                       + ordFileName );
                    System.exit(1);
                }
            try
                {
                    favoredEdgeReader
                        = new Scanner( new FileReader( favoredEdgeFileName ) );
                }
            catch (IOException exception)
                {
                    System.out.println("Unable to read file " + favoredEdgeFileName );
                    System.exit(1);
                }
            System.out.println( "Favored edges:" );
            favoredEdges = initDag( favoredEdgeReader, ordReader );
            if ( DEBUG )
                System.out.printf( "Favored edge graph:\n%s\n", favoredEdges );
       
            ordReader.close();
            favoredEdgeReader.close();
        }
    }
    
 
   /**
     * Handles commandline arguments, opens files, and initiates drawing
     */
    public static void main(String [] args)
    {
        if( args.length < 6 )
            {
                printUsage();
                return;
            }
        String dotFileName = args[0];
        String ordFileName = args[1];
        String favoredEdgeFileName = args[2];
        int width = Integer.parseInt(args[3]);
        int height = Integer.parseInt(args[4]);
        fontSize = Integer.parseInt(args[5]);
        if( width <= 3 * MARGIN || height <= 3 * MARGIN )
            {
                System.out.println("Width and height must each be > "
                                   + (3 * MARGIN) );
                return;
            }
        if( fontSize <= 0 )
            noLabels = true;

        DagDisplay display = new DagDisplay( "Layered Graph", width, height );
        
        display.readDags( dotFileName, ordFileName, favoredEdgeFileName );
        display.addCanvas();

    } // end, main
} // end, class

//  [Last modified: 2014 01 28 at 20:42:36 GMT]
