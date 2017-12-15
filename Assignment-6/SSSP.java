/*
    SSSP.java
    Single-source shortest path finder.
    Includes a (sequential) implementation of Dijkstra's algorithm,
    which is O((m + n) log n).
    Also includes a (sequential) implementation of Delta stepping.
    You need to create a parallel version of this.
    Michael L. Scott, November 2017; based heavily on earlier
    incarnations of several programming projects, and on Delaunay mesh
    code written in 2007.
 */

//package csc254;

import java.awt.*;
import java.awt.event.*;
import java.io.*;
import javax.swing.*;

import java.util.concurrent.TimeUnit;

import java.util.*;
import java.util.concurrent.BrokenBarrierException;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.CyclicBarrier;
import java.lang.*;

public class SSSP {
    private static int n = 50;              // default number of vertices
    private static double geom = 1.0;       // default degree of geometric reality
        // 0 means random edge weight; 1 means fully geometric distance
    private static int degree = 5;          // expected number of neighbors per vertex
                                            // (near the middle of the graph)
    private static long sd = 0;             // default random number seed
    private static int numThreads = 0;      // zero means use Dijkstra's alg;
                                            // positive means use Delta stepping

    private static final int TIMING_ONLY    = 0;
    private static final int PRINT_EVENTS   = 1;
    private static final int SHOW_RESULT    = 2;
    private static final int FULL_ANIMATION = 3;
    private static int animate = TIMING_ONLY;       // default

    private static final String help =
            "-a [0123] annimation mode:\n"
          + "    0 -> timing only\n"
          + "    1 -> print events only\n"
          + "    2 -> show result\n"
          + "    3 -> full animation\n"
          + "-n <number of vertices>\n"
          + "-d <expected vertex degree>\n"
          + "    (for vertices near the middle of large graphs)\n"
          + "-g <degree of geometric realism>\n"
          + "    (real number between 0 and 1)\n"
          + "-s <random number seed>\n"
          + "-t <number of threads>\n"
          + "    (0 means use Dijkstra's algorithm on one thread)\n"
          + "-v  (print this message)\n";

    // Examine command-line arguments for alternative running modes.
    //
    private static void parseArgs(String[] args) {
        for (int i = 0; i < args.length; i++) {
            if (args[i].equals("-a")) {
                if (++i >= args.length) {
                    System.err.print("Missing animation level\n");
                } else {
                    int an = -1;
                    try {
                        an = Integer.parseInt(args[i]);
                    } catch (NumberFormatException e) { }
                    if (an >= TIMING_ONLY && an <= FULL_ANIMATION) {
                        animate = an;
                    } else {
                        System.err.printf("Animation level (%s) must be between 0 and 3.\n",
                            args[i]);
                    }
                }
            } else if (args[i].equals("-n")) {
                if (++i >= args.length) {
                    System.err.print("Missing number of vertices\n");
                } else {
                    int np = -1;
                    try {
                        np = Integer.parseInt(args[i]);
                    } catch (NumberFormatException e) { }
                    if (np > 0) {
                        n = np;
                    } else {
                        System.err.printf("Number of vertices (%s) must be positive.\n",
                            args[i]);
                    }
                }
            } else if (args[i].equals("-d")) {
                if (++i >= args.length) {
                    System.err.print("Missing degree\n");
                } else {
                    int d = -1;
                    try {
                        d = Integer.parseInt(args[i]);
                    } catch (NumberFormatException e) { }
                    if (d > 0) {
                        degree = d;
                    } else {
                        System.err.printf("Expected degree (%s) must be positive.\n",
                            args[i]);
                    }
                }
            } else if (args[i].equals("-g")) {
                if (++i >= args.length) {
                    System.err.print("Missing geometry factor\n");
                } else {
                    double g = -1.0;
                    try {
                        g = Double.parseDouble(args[i]);
                    } catch (NumberFormatException e) { }
                    if (g >= 0 && g <= 1) {
                        geom = g;
                    } else {
                        System.err.printf("Geometry factor (%s) must be between 0 and 1.\n",
                            args[i]);
                    }
                }
            } else if (args[i].equals("-s")) {
                if (++i >= args.length) {
                    System.err.print("Missing seed\n");
                } else {
                    try {
                        sd = Long.parseLong(args[i]);
                    } catch (NumberFormatException e) {
                        System.err.printf("Seed (%s) must be a long integer\n", args[i]);
                    }
                }
            } else if (args[i].equals("-t")) {
                if (++i >= args.length) {
                    System.err.print("Missing number of threads\n");
                } else {
                    int nt = -1;
                    try {
                        nt = Integer.parseInt(args[i]);
                    } catch (NumberFormatException e) { }
                    if (nt >= 0) {
                        numThreads = nt;
                    } else {
                        System.err.printf("Number of threads (%s) must be nonnegative.\n",
                            args[i]);
                    }
                }
            } else if (args[i].equals("-v")) {
                System.err.print(help);
                System.exit(0);
            } else {
                System.err.printf("Unexpected argument: %s\n", args[i]);
                System.err.print(help);
                System.exit(1);
            }
        }
    }

    // Initialize appropriate program components for specified animation mode.
    //
    private Surface build(RootPaneContainer pane, int an) {
        final Coordinator c = new Coordinator();
        Surface s = new Surface(n, sd, geom, degree, c);
        Animation at = null;
        if (an == SHOW_RESULT || an == FULL_ANIMATION) {
            at = new Animation(s);
            new UI(c, s, at, sd, numThreads, pane);
        }
        final Animation a = at;
        if (an == PRINT_EVENTS) {
            s.setHooks(
                new Surface.EdgeRoutine() {
                    public void run(int x1, int y1, int x2, int y2, boolean dum, long w) {
                        System.out.printf("selected  %12d %12d %12d %12d %12d\n",
                                          x1, y1, x2, y2, w);
                    }},
                new Surface.EdgeRoutine() {
                    public void run(int x1, int y1, int x2, int y2, boolean dum, long w) {
                        System.out.printf("unselected  %12d %12d %12d %12d %12d\n",
                                          x1, y1, x2, y2, w);
                    }});
        } else if (an == FULL_ANIMATION) {
            Surface.EdgeRoutine er = new Surface.EdgeRoutine() {
                public void run(int x1, int y1, int x2, int y2, boolean dum, long w)
                        throws Coordinator.KilledException {
                    c.hesitate();
                    a.repaint();        // graphics need to be re-rendered
                }};
            s.setHooks(er, er);
        }
        return s;
    }

    public static void main(String[] args) {
        parseArgs(args);
        SSSP me = new SSSP();
        JFrame f = null;
        if (animate == SHOW_RESULT || animate == FULL_ANIMATION) {
            f = new JFrame("SSSP");
            f.addWindowListener(new WindowAdapter() {
                public void windowClosing(WindowEvent e) {
                    System.exit(0);
                }
            });
        } else {
            System.out.printf("%d vertices, seed %d\n", n, sd);
        }
        Surface s = me.build(f, animate);
        if (f != null) {
            f.pack();
            f.setVisible(true);
        } else {
            // Using terminal I/O rather than graphics.
            // Execute the guts of the run button handler method here.
            long startTime = new Date().getTime();
            try {
                if (numThreads == 0) {
                    s.DijkstraSolve();
                } else {
                	CyclicBarrier barrier = new CyclicBarrier(numThreads);
                    s.DeltaSolve(numThreads, barrier);
                }
            } catch(Coordinator.KilledException e) { }
            long endTime = new Date().getTime();
            System.out.printf("elapsed time: %.3f seconds\n",
                              (double) (endTime-startTime)/1000);
        }
    }
}

// The Worker is the thread that does the actual work of finding
// shortest paths (in the animated version -- main thread does it in
// the terminal I/O version).
//
class Worker extends Thread {
    private final Surface s;
    private final Coordinator c;
    private final UI u;
    private final Animation a;
    private final boolean dijkstra;     // Dijkstra = !Delta

    // The run() method of a Java Thread is never invoked directly by
    // user code.  Rather, it is called by the Java runtime when user
    // code calls start().
    //
    // The run() method of a worker thread *must* begin by calling
    // c.register() and end by calling c.unregister().  These allow the
    // user interface (via the Coordinator) to pause and terminate
    // workers.  Note how the worker is set up to catch KilledException.
    // In the process of unwinding back to here we'll cleanly and
    // automatically release any monitor locks.  If you create new kinds
    // of workers (as part of a parallel solver), make sure they call
    // c.register() and c.unregister() properly.
    //
    public void run() {
        try {
            c.register();
            if (dijkstra) {
                s.DijkstraSolve();
            } else {
            	CyclicBarrier barrier = new CyclicBarrier(1);
                s.DeltaSolve(1, barrier);
            }
            c.unregister();
        } catch(Coordinator.KilledException e) { }
        if (a != null) {
            // Tell the graphics event thread to unset the default
            // button when it gets a chance.  (Threads other than the
            // event thread cannot safely modify the GUI directly.)
            a.repaint();
            SwingUtilities.invokeLater(new Runnable() {
                public void run() {
                    u.setDone();
                }
            });
        }
    }

    // Constructor
    //
    public Worker(Surface S, Coordinator C, UI U, Animation A, boolean D) {
        s = S;
        c = C;
        u = U;
        a = A;
        dijkstra = D;
    }
}

// The Surface is the SSSP world, containing all the vertices.
// Vertex 0 is the source.
//
class Surface {
    // all X and Y coordinates will be in the range [0..2^28)
    public static final int minCoord = 0;
    public static final int maxCoord = 1024*1024*256;

    // The following 9 fields are set by the Surface constructor.
    private final Coordinator coord;
        // Not needed at present, but will need to be passed to any
        // newly created workers.
    private final int n;  // number of vertices
    private final Vertex vertices[];
        // Main array of vertices, used for partitioning and rendering.
    private final HashSet<Vertex> vertexHash;
        // Used to ensure that we never have two vertices directly on top of
        // each other.  See Vertex.hashCode and Vertex.equals below.
    private final Vector<Edge> edges;
    private long sd = 0;
    private double geom;        // degree of geometric realism
    private int degree;         // desired average node degree
    private final Random prn;   // pseudo-random number generator

    private class Vertex {
        public final int xCoord;
        public final int yCoord;

        public Vector<Edge> neighbors;

        public long distToSource;
        public Edge predecessor;

        // Add a new neighbor to this vertex (called only during initialization)
        public void addNeighbor(Edge e) {
            neighbors.add(e);
        }

        // Override Object.hashCode and Object.equals.
        // This way two vertices are equal (and hash to the same slot in
        // HashSet vertexHash) if they have the same coordinates, even if they
        // are different objects.
        //
        public int hashCode() {
            return xCoord ^ yCoord;
        }
        public boolean equals(Object o) {
            Vertex v = (Vertex) o;            // run-time type check
            return v.xCoord == xCoord && v.yCoord == yCoord;
        }

        // Constructor
        //
        public Vertex(int x, int y) {
            xCoord = x;  yCoord = y;
            neighbors = new Vector<Edge>();
            distToSource = Long.MAX_VALUE;
            predecessor = null;
        }
    }

    // Distance-based Comparator for use in Dijkstra priority queue.
    // Note: this comparator imposes orderings that are inconsistent with equals.
    // More specifically: v1.equals(v2) => compare(v1, v2) == 0, but it is
    // not necessarily the case that compare(v1, v2) == 0 => v1.equals(v2).
    //
    private class DistanceComparator implements Comparator<Vertex> {
        public int compare(Vertex v1, Vertex v2) {
            if (v1.distToSource < v2.distToSource) return -1;
            if (v2.distToSource < v1.distToSource) return 1;
            return 0;
        }
    }

    // In a purely offline SSSP algorithm we probably wouldn't need an
    // explicit edge class.  Having one makes the graphics a lot more
    // straightforward, though.
    //
    private class Edge {
        public final Vertex v1;  // vertices are in arbitrary order
        public final Vertex v2;
        public final int weight;
        private boolean selected;

        public void select() throws Coordinator.KilledException {
            selected = true;
            if (edgeSelectHook != null) {
                edgeSelectHook.run(v1.xCoord, v1.yCoord, v2.xCoord, v2.yCoord, true,
                       Math.max(v1.distToSource, v2.distToSource));
            }
        }

        public void unselect() throws Coordinator.KilledException {
            selected = false;
            if (edgeUnSelectHook != null) {
                edgeUnSelectHook.run(v1.xCoord, v1.yCoord, v2.xCoord, v2.yCoord, false, 0);
            }
        }

        public Vertex other(Vertex v) {
            if (v == v1) {
                return v2;
            } else {
                return v1;
            }
        }

        // Constructor
        //
        public Edge(Vertex first, Vertex second, int w) {
            v1 = first;  v2 = second;  weight = w;  selected = false;
        }
    }

    // Signatures for things someone might want us to do with a vertex or
    // an edge (e.g., display it).
    //
    public interface EdgeRoutine {
        public void run(int x1, int y1, int x2, int y2, boolean selected, long weight)
            throws Coordinator.KilledException;
    }
    public interface VertexRoutine{
        public void run(int x, int y);
    }

    public void forAllVertices(VertexRoutine pr) {
        for (Vertex v : vertices) {
            pr.run(v.xCoord, v.yCoord);
        }
    }

    public void forSource(VertexRoutine pr) {
        pr.run(vertices[0].xCoord, vertices[0].yCoord);
    }

    public void forAllEdges(EdgeRoutine pr) {
        for (Edge e : edges) {
            try {
                pr.run(e.v1.xCoord, e.v1.yCoord, e.v2.xCoord, e.v2.yCoord, e.selected, 0);
            } catch (Coordinator.KilledException f) { }
        }
    }

    // Routines to call when performing the specified operations:
    private static EdgeRoutine edgeSelectHook = null;
    private static EdgeRoutine edgeUnSelectHook = null;

    // The following is separate from the constructor to avoid a
    // circularity problem: when working in FULL_ANIMATION mode, the
    // Animation object needs a reference to the Surface object, and the
    // Surface object needs references to the hooks of the Animation object.
    //
    public void setHooks(EdgeRoutine esh, EdgeRoutine euh) {
        edgeSelectHook = esh;
        edgeUnSelectHook = euh;
    }

    // Called by the UI when it wants to reset with a new seed.
    //
    public long randomize() {
        sd++;
        reset();
        return sd;
    }

    // Compute Euclidean distance between two vertices.
    //
    private int euclideanDistance(Vertex v1, Vertex v2) {
        double xDiff = v1.xCoord - v2.xCoord;
        double yDiff = v1.yCoord - v2.yCoord;
        return (int) Math.sqrt(xDiff * xDiff + yDiff * yDiff);
    }

    // 2-dimensional array of buckets into which to put geometrically
    // proximal vertices.  Sadly, requires suppression of unchecked cast
    // warnings.  (I could get around that with a an ArrayList of
    // ArrayLists, but that gets really messy...)
    //
    class CheckerBoard {
        private Object[][] cb;
        @SuppressWarnings("unchecked")
        public Vector<Vertex> get(int i, int j) {
            return (Vector<Vertex>)(cb[i][j]);
        }
        public CheckerBoard (int k) {
            cb = new Object[k][k];
            // Really Vector<Vertex>, but Java erasure makes that illegal.
            for (int i = 0; i < k; ++i) {
                for (int j = 0; j < k; ++j) {
                    cb[i][j] = new Vector<Vertex>();
                }
            }
        }
    }

    // Called by the UI when it wants to start over.
    //
    public void reset() {
        // As a heuristic, I want to connect each vertex to about 1/4 of
        // its geometrically nearby vertices.  So I want to choose
        // neighbors from a region containing about 4*degree vertices.
        // I divide the plane into a k x k grid, such that a 3x3 subset
        // has about the right number of vertices from which to choose.
        final int k = (int) (Math.sqrt((double)n/(double)degree) * 3 / 2);
        final int sw = (int) Math.ceil((double)maxCoord/(double)k);     // square width;
        CheckerBoard cb = new CheckerBoard(k);

        prn.setSeed(sd);
        vertexHash.clear();     // empty out the set of vertices
        edges.clear();          // and edges
        for (int i = 0; i < n; i++) {
            Vertex v;
            int x;
            int y;
            do {
                x = Math.abs(prn.nextInt()) % maxCoord;
                y = Math.abs(prn.nextInt()) % maxCoord;
                v = new Vertex(x, y);
            } while (vertexHash.contains(v));
            vertexHash.add(v);
            vertices[i] = v;
            cb.get(x/sw, y/sw).add(v);
        }
        vertices[0].distToSource = 0;   // vertex 0 is the source

        // create edges
        for (Vertex v : vertices) {
            int xb = v.xCoord / sw;
            int yb = v.yCoord / sw;
            // Find 3x3 area from which to draw neighbors.
            int xl;  int xh;
            int yl;  int yh;
            if (k < 3) {
                xl = yl = 0;
                xh = yh = k-1;
            } else {
                xl = (xb == 0) ? 0 : ((xb == k-1) ? k-3 : (xb-1));
                xh = (xb == 0) ? 2 : ((xb == k-1) ? k-1 : (xb+1));
                yl = (yb == 0) ? 0 : ((yb == k-1) ? k-3 : (yb-1));
                yh = (yb == 0) ? 2 : ((yb == k-1) ? k-1 : (yb+1));
            }
            for (int i = xl; i <= xh; ++i) {
                for (int j = yl; j <= yh; ++j) {
                    for (Vertex u : cb.get(i, j)) {
                        if (v.hashCode() < u.hashCode()
                                // Only choose edge from one end --
                                // avoid self-loops and doubled edges.
                                && prn.nextInt() % 4 == 0) {
                            // Invent a weight.
                            int dist = euclideanDistance(u, v);
                            int randWeight = Math.abs(prn.nextInt()) % (maxCoord * 2);
                            int weight = (int) ((geom * (double)dist)
                                                + ((1.0 - geom) * (double)randWeight));
                            // Pick u as neighbor.
                            Edge e = new Edge(u, v, weight);
                            u.addNeighbor(e);
                            v.addNeighbor(e);
                            edges.add(e);
                        }
                    }
                }
            }
        }
    }

    // *************************
    // Find shortest paths via Dijkstra's algorithm.
    //
/*
    public void DijkstraSolve() throws Coordinator.KilledException {
        PriorityQueue<Vertex> pq = new PriorityQueue<Vertex>(n, new DistanceComparator());
        Vertex v = vertices[0];
        for (Edge e : v.neighbors) {
            Vertex o = e.other(v);
            o.distToSource = e.weight;
            o.predecessor = e;
        }
        for (int i = 1; i < n; i++) {   // don't bother adding source
            pq.add(vertices[i]);
        }
        while (!pq.isEmpty()) {
            v = pq.poll();
            if (v.distToSource == Long.MAX_VALUE) {
                // this is a disconnected vertex, as are all that remain
                break;
            }
            v.predecessor.select();
            for (Edge e : v.neighbors) {
                Vertex o = e.other(v);
                long altDist = v.distToSource + e.weight;
                // relax (o, altDist)
                if (altDist < o.distToSource) {
                    pq.remove(o);
                    o.distToSource = altDist;
                    o.predecessor = e;
                    pq.add(o);
                }
            }
        }
        //  print results
    }
*/
	
    class WeightedVertex implements Comparable<WeightedVertex> {
        Vertex v;
        long weight;

        public WeightedVertex(Vertex n) {
            v = n;
            weight = v.distToSource;
        }

        public int compareTo(WeightedVertex other) {
            if (weight < other.weight) return -1;
            if (weight == other.weight) return 0;
            return 1;
        }
    }

    public void DijkstraSolve() throws Coordinator.KilledException {
        PriorityQueue<WeightedVertex> pq =
            new PriorityQueue<WeightedVertex>((n * 12) / 10);
            // Leave some room for extra umremoved entries.
        vertices[0].distToSource = 0;
        // All other vertices still have maximal distToSource, as set by constructor.
        pq.add(new WeightedVertex(vertices[0]));
        while (!pq.isEmpty()) {
            WeightedVertex wv = pq.poll();
            Vertex v = wv.v;
            if (v.predecessor != null) {
                v.predecessor.select();
            }
            if (wv.weight != v.distToSource) {
                // This is a left-over pq entry.
                continue;
            }
            for (Edge e : v.neighbors) {
                Vertex o = e.other(v);
                long altDist = v.distToSource + e.weight;
                if (altDist < o.distToSource) {
                    o.distToSource = altDist;
                    o.predecessor = e;
                    pq.add(new WeightedVertex(o));
                }
            }
        }
    }
    
    // *************************
    // Find shortest paths via Delta stepping.

    int numBuckets;
    int delta;
    private ArrayList<LinkedHashSet<Vertex>> buckets;
    // This is an ArrayList instead of a plain array to avoid the generic
    // array creation error message that stems from Java erasure.

    // A Request is a potential relaxation.
    //
    class Request {
        private Vertex v;
        private Edge e;

        // To relax a request is to consider whether the e might provide
        // v with a better path back to the source.
        //
        public void relax(Vector<LinkedHashSet<Vertex>> buckets) throws Coordinator.KilledException {
            Vertex o = e.other(v);
            long altDist = o.distToSource + e.weight;
            if (altDist < v.distToSource) {
                // Yup; better path home.
                buckets.get((int)((v.distToSource / delta) % numBuckets)).remove(v);
                v.distToSource = altDist;
                if (v.predecessor != null) {
                    v.predecessor.unselect();
                }
                v.predecessor = e;
                e.select();
                buckets.get((int)((altDist / delta) % numBuckets)).add(v);
            }
        }

        public Request(Vertex V, Edge E) {
            v = V;  e = E;
        }
        
        public Vertex getV() {
        	return v;
        }
    }

    // Return list of requests whose connecting edge weight is <= or > than delta.
    //
    LinkedList<Request> findRequests(Collection<Vertex> bucket, boolean light) {
        LinkedList<Request> rtn = new LinkedList<Request>();
        for (Vertex v : bucket) {
            for (Edge e : v.neighbors) {
                if ((light && e.weight <= delta) || (!light && e.weight > delta)) {
                    Vertex o = e.other(v);
                    rtn.add(new Request(o, e));
                }
            }
        }
        return rtn;
    }
    
    //  map vertex to thread
    HashMap<Vertex, Long> v2t = new HashMap<Vertex, Long>();
    
    //  map thread to integer
    HashMap<Long, Integer> t2i = new HashMap<Long, Integer>();
    
    //  check the current bucket is done
    ArrayList<Boolean> isEmpty = new ArrayList<Boolean>();
    
    //  check the whole buckets is empty
    ArrayList<Boolean> isDone = new ArrayList<Boolean>();
    
    //  message queue
    ArrayList<ConcurrentLinkedQueue<Request>> msgQ = new ArrayList<ConcurrentLinkedQueue<Request>>();
    
    class ThreadWorker extends Thread {
    	private int numBuckets;
    	private CyclicBarrier barrier;
    	private Vector<LinkedHashSet<Vertex>> buckets;
    	private int i;
    	
    	public ThreadWorker(int numBuckets, int delta, CyclicBarrier barrier) {
    		this.numBuckets = numBuckets;
    		this.barrier = barrier;
    		this.buckets = new Vector<LinkedHashSet<Vertex>>(numBuckets);
    		this.i = 0;
    		for (int i = 0; i < numBuckets; i++)
    			buckets.add(new LinkedHashSet<Vertex>());
            buckets.get(0).add(vertices[0]);
    	}
    	
    	
    	public void run() {

            
    		while (true) {
    			
    			while (true) {

    				LinkedList<Vertex> removed = new LinkedList<Vertex>();
    				LinkedList<Request> requests;
    				    				
    				//  deal with light 
    				while (buckets.get(i).size() > 0) {
    					requests = findRequests(buckets.get(i), true);
    					
    					removed.addAll(buckets.get(i));
    					buckets.set(i, new LinkedHashSet<Vertex>());
    					
    					for (Request rq : requests) {
    						if (v2t.get(rq.getV()) == Thread.currentThread().getId()) {
									try {
										rq.relax(buckets);
									} catch (Coordinator.KilledException e) {
										// TODO Auto-generated catch block
										e.printStackTrace();
									}
    						} else {
    							long srcThreadId = Thread.currentThread().getId();
    							long dstThreadId = v2t.get(rq.getV());
    							int srcThreadInt = t2i.get(srcThreadId);
    							int dstThreadInt = t2i.get(dstThreadId);
    							
    							msgQ.get(dstThreadInt).add(rq);   							
    						}
    					}
    				}    					


									
					
					//  deal received request
					int curThreadInt = t2i.get(Thread.currentThread().getId());


					//  2nd barrier
                    try {
                        barrier.await();
                    } catch (InterruptedException e) {
                        // TODO Auto-generated catch block
                        e.printStackTrace();
                    } catch (BrokenBarrierException e) {
                        // TODO Auto-generated catch block
                        e.printStackTrace();
                    }

					//  find heavy
					requests = findRequests(removed, false);
					for (Request rq : requests) {
						if (v2t.get(rq.getV()) == Thread.currentThread().getId()) {
							try {
								rq.relax(buckets);
							} catch (Coordinator.KilledException e) {
								// TODO Auto-generated catch block
								e.printStackTrace();
							}
						} else {
							long srcThreadId = Thread.currentThread().getId();
							long dstThreadId = v2t.get(rq.getV());
							int srcThreadInt = t2i.get(srcThreadId);
							int dstThreadInt = t2i.get(dstThreadId);
							msgQ.get(dstThreadInt).add(rq);
						}
					}
					
					

					
					//  deal heavy
					while (!msgQ.get(curThreadInt).isEmpty()) {
						Request rq = msgQ.get(curThreadInt).poll();
						try {
							rq.relax(buckets);
						} catch (Coordinator.KilledException e) {
							// TODO Auto-generated catch block
							e.printStackTrace();
						}
					}				
	

					
					
					if (buckets.get(i).size() == 0) {
						isEmpty.set(t2i.get(Thread.currentThread().getId()), true);
					} else {
						isEmpty.set(t2i.get(Thread.currentThread().getId()), false);
					}
					
					
					
				//  5th barrier
                    try {
                        barrier.await();
                    } catch (InterruptedException e) {
                        // TODO Auto-generated catch block
                        e.printStackTrace();
                    } catch (BrokenBarrierException e) {
                        // TODO Auto-generated catch block
                        e.printStackTrace();
                    }
					
					boolean f = true;
					for (int i = 0; i < isEmpty.size(); i++) {
						f &= isEmpty.get(i);
					}
					
					if (f)
						break;
    			}
    			//  System.out.println(Thread.currentThread().getId() + "\t" + i);
    			i = (i + 1) % numBuckets;
    			

    			    			
    			
    			
    			boolean qEmpty = true;
    			for (int i = 0; i < numBuckets; i++) {
    				if (buckets.get(i).isEmpty() == false)
    					qEmpty = false;
    			}
    			
    			if (qEmpty) {
    				isDone.set(t2i.get(Thread.currentThread().getId()), true);
    			} else {
    				isDone.set(t2i.get(Thread.currentThread().getId()), false);
    			}
    			
    			try {
					barrier.await();
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				} catch (BrokenBarrierException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
    			
    			boolean f = true;
    			for (int i = 0; i < isDone.size(); i++) {
    				f &= isDone.get(i);
    			}
    			
    			if (f)
    				break;
    		}
    	}
    }
    
    // Main solver routine.
    //
    public void DeltaSolve(int numThreads, CyclicBarrier barrier) throws Coordinator.KilledException {
        numBuckets = 2 * degree;
        delta = maxCoord / degree;
        // All buckets, together, cover a range of 2 * maxCoord,
        // which is larger than the weight of any edge, so a relaxation
        // will never wrap all the way around the array.
              
        
        //  set isDone to false
        for (int i = 0; i < numThreads; i++) {
        	isEmpty.add(true);
        }
        
        for (int i = 0; i < numThreads; i++) {
        	isDone.add(true);
        }
        
        //  initialize message queue
        for (int i = 0; i < numThreads; i++) {
        	msgQ.add(new ConcurrentLinkedQueue<Request>());
        }
        //  thread pool
        Vector<ThreadWorker> threadPool = new Vector<ThreadWorker>();
        for (int i = 0; i < numThreads; i++) {
        	threadPool.add(new ThreadWorker(numBuckets, delta, barrier));
        }
        
        //  assign vertex to thread
        for (int i = 0; i < n; i++) {
        	v2t.put(vertices[i], threadPool.get(i % numThreads).getId());
        }
        
        //  assign thread to integer
        for (int i = 0; i < numThreads; i++) {
        	t2i.put(threadPool.get(i).getId(), i);
        }
        long lStartTime = System.currentTimeMillis();
        //  start thread
        for (int i = 0; i < numThreads; i++) {
        	threadPool.get(i).start();
        }
        
        //  main thread waits all worker threads
        for (int i = 0; i < numThreads; i++) {
        	try {
				threadPool.get(i).join();
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
        }
        
    
        
        long lEndTime = System.currentTimeMillis();
        
        long output = lEndTime - lStartTime;
        
        System.out.println("Elapsed time in milliseconds: " + output);
//        //  print results
    }

    // End of Delta stepping.
    // *************************
    
    
    // Constructor
    //
    public Surface(int N, long SD, double G, int D, Coordinator C) {
        n = N;
        sd = SD;
        geom = G;
        degree = D;
        coord = C;

        vertices = new Vertex[n];
        vertexHash = new HashSet<Vertex>(n);
        edges = new Vector<Edge>();

        prn = new Random();
        reset();
    }
}

// Class Animation is the one really complicated sub-pane of the user interface.
//
class Animation extends JPanel {
    private static final int width = 512;      // canvas dimensions
    private static final int height = 512;
    private static final int dotsize = 6;
    private static final int border = dotsize;
    private final Surface s;

    // The next two routines figure out where to render the dot
    // for a vertex, given the size of the animation panel and the spread
    // of x and y values among all vertices.
    //
    private int xPosition(int x) {
        return (int)
            (((double)x) * (double)width / (double)s.maxCoord) + border;
    }
    private int yPosition(int y) {
        return (int)
            (((double)s.maxCoord - (double)y) * (double)height
                / ((double)s.maxCoord)) + border;
    }

    // The following method is called automatically by the graphics
    // system when it thinks the Animation canvas needs to be
    // re-displayed.  This can happen because code elsewhere in this
    // program called repaint(), or because of hiding/revealing or
    // open/close operations in the surrounding window system.
    //
    public void paintComponent(final Graphics g) {
        final Graphics2D g2 = (Graphics2D) g;

        super.paintComponent(g);    // clears panel
        s.forAllEdges(new Surface.EdgeRoutine() {
            public void run(int x1, int y1, int x2, int y2, boolean bold, long w) {
                if (bold) {
                    g2.setPaint(Color.red);
                    g2.setStroke(new BasicStroke(3));
                } else {
                    g2.setPaint(Color.gray);
                    g2.setStroke(new BasicStroke(1));
                }
                g.drawLine(xPosition(x1), yPosition(y1),
                           xPosition(x2), yPosition(y2));
            }
        });
        s.forAllVertices(new Surface.VertexRoutine() {
            public void run(int x, int y) {
                g2.setPaint(Color.blue);
                g.fillOval(xPosition(x)-dotsize/2, yPosition(y)-dotsize/2,
                           dotsize, dotsize);
            }
        });
        // Distinguish source vertex:
        s.forSource(new Surface.VertexRoutine() {
            public void run(int x, int y) {
                g2.setPaint(Color.green);
                g.fillOval(xPosition(x)-dotsize, yPosition(y)-dotsize,
                           dotsize*2, dotsize*2);
                g2.setPaint(Color.black);
                g2.setStroke(new BasicStroke(2));
                g.drawOval(xPosition(x)-dotsize, yPosition(y)-dotsize,
                           dotsize*2, dotsize*2);
            }
        });
    }

    // UI needs to call this routine when vertex locations have changed.
    //
    public void reset() {
        repaint();      // Tell graphics system to re-render.
    }

    // Constructor
    //
    public Animation(Surface S) {
        setPreferredSize(new Dimension(width+border*2, height+border*2));
        setBackground(Color.white);
        setForeground(Color.black);
        s = S;
        reset();
    }
}

// Class UI is the user interface.  It displays a Surface canvas above
// a row of buttons and a row of statistics.  Actions (event handlers)
// are defined for each of the buttons.  Depending on the state of the
// UI, either the "run" or the "pause" button is the default (highlighted in
// most window systems); it will often self-push if you hit carriage return.
//
class UI extends JPanel {
    private final Coordinator coordinator;
    private final Surface surface;
    private final Animation animation;

    private final JRootPane root;
    private static final int externalBorder = 6;

    private static final int stopped = 0;
    private static final int running = 1;
    private static final int paused = 2;
    private static final int done = 3;

    private int state = stopped;
    private long elapsedTime = 0;
    private long startTime;

    private final JLabel time = new JLabel("time: 0");

    public void updateTime() {
        Date d = new Date();
        elapsedTime += (d.getTime() - startTime);
        time.setText(String.format("time: %d.%03d", elapsedTime/1000,
                                                    elapsedTime%1000));
    }

    public void setDone() {
        root.setDefaultButton(null);
        updateTime();
        state = done;
    };

    // Constructor
    //
    public UI(Coordinator C, Surface S, Animation A,
              long SD, int NT, RootPaneContainer pane) {
        final UI ui = this;
        coordinator = C;
        surface = S;
        animation = A;

        final JPanel buttons = new JPanel();   // button panel
            final JButton runButton = new JButton("Run");
            final JButton pauseButton = new JButton("Pause");
            final JButton resetButton = new JButton("Reset");
            final JButton randomizeButton = new JButton("Randomize");
            final JButton quitButton = new JButton("Quit");

        final JPanel stats = new JPanel();   // statistics panel

        final JLabel seed = new JLabel("seed: " + SD + "   ");

        runButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                if (state == stopped) {
                    state = running;
                    root.setDefaultButton(pauseButton);
                    Worker w = new Worker(surface, coordinator,
                                          ui, animation, NT == 0);
                    Date d = new Date();
                    startTime = d.getTime();
                    w.start();
                } else if (state == paused) {
                    state = running;
                    root.setDefaultButton(pauseButton);
                    Date d = new Date();
                    startTime = d.getTime();
                    coordinator.toggle();
                }
            }
        });
        pauseButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                if (state == running) {
                    updateTime();
                    state = paused;
                    root.setDefaultButton(runButton);
                    coordinator.toggle();
                }
            }
        });
        resetButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                state = stopped;
                coordinator.stop();
                root.setDefaultButton(runButton);
                surface.reset();
                animation.reset();
                elapsedTime = 0;
                time.setText("time: 0");
            }
        });
        randomizeButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                state = stopped;
                coordinator.stop();
                root.setDefaultButton(runButton);
                long v = surface.randomize();
                animation.reset();
                seed.setText("seed: " + v + "   ");
                elapsedTime = 0;
                time.setText("time: 0");
            }
        });
        quitButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                System.exit(0);
            }
        });

        // Put the buttons into the button panel:
        buttons.setLayout(new FlowLayout());
        buttons.add(runButton);
        buttons.add(pauseButton);
        buttons.add(resetButton);
        buttons.add(randomizeButton);
        buttons.add(quitButton);

        // Put the labels into the statistics panel:
        stats.add(seed);
        stats.add(time);

        // Put the Surface canvas, the button panel, and the stats
        // label into the UI:
        setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
        setBorder(BorderFactory.createEmptyBorder(externalBorder,
            externalBorder, externalBorder, externalBorder));
        add(A);
        add(buttons);
        add(stats);

        // Put the UI into the Frame:
        pane.getContentPane().add(this);
        root = getRootPane();
        root.setDefaultButton(runButton);
    }
}
