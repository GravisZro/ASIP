==================================================
ASIP: Approximating Shapes in Images with Polygons
==================================================

This code, licensed by Inria, is part of the following publication:
M. Li, F. Lafarge, R. Marlet, Approximating shapes in images with low-complexity polygons, IEEE/CVF Conference
on Computer Vision and Pattern Recognition (CVPR), 2020

Source code: https://team.inria.fr/titane/software/


---------------------
Dependencies (add links to each library)
---------------------

- CMake (tested with CMake 3.16)

- Third Party Libraries:
1. Boost (required components: filesystem): https://www.boost.org/
2. Eigen3: http://eigen.tuxfamily.org/index.php?title=Main_Page#Download
3. OpenCV: https://opencv.org/releases/
4. GDAL: https://gdal.org/download.html#current-releases
5. CGAL: https://www.cgal.org/
6. Line Segment Detector (code already included): http://www.ipol.im/pub/art/2012/gjmr-lsd/

- Optional Third Party Library:
1. Qt5 (for GUI): https://www.qt.io/download


---------------------
Compile with CMake
---------------------
After installing all dependencies*, for linux simply use
mkdir build
cd build
cmake ..
make

*For Windows users, it is recommended to use a package manager such as vcpkg for installing dependencies. Otherwise you may need to mannually edit environment variables and provide paths in CMake GUI.


---------------------
How to run
---------------------

-------Console-------
Run the console executable in terminal, a helper message will be displayed for explaining input arguments.

The minimal arguments required are:
--input                          Specify an input image (TIF, JPEG, PNG)
--input-probability              Specify an input probability map (TIF, one channel per class)
--output-directory               Specify an output directory
--iterations                     Maximum number of iterations

Optional arguments:
--fidelity-beta                  The weight of edge alignment (default: 0.0002)
--prior-lambda                   The weight on the complexity term (default: 0.000001)
--verbose-level                  Specify the verbose level for console display (default: 3)

Example console call:
./image_vectorization_console --input ../data/example_hku-is/0012.png --input-probability ../data/example_hku-is/0012_dcl_crf.png --output-directory ./output --iterations 2000 --fidelity-beta 0.0002 --prior-lambda 0.000001 


-------GUI-------
You need to provide at least an input image and a probability map (OR a pixel-wise semantic label map). The GUI does not support GeoTIFF format, for which please use the console application. 

List of options:
Alignment weight                          The beta parameter as described in the paper, which encourages polygon edges to align with image gradients
Complexity weight                         The lambda parameter as described in the paper, which encourages simple polygons with fewer edges
Minimum length for line detection         The algorithm will try to ignore line segments shorter than the specified value. When set to 0, the algorithm will compare with 0.02*image_diagonal_length
Number of iterations                      Maximum number of iterations


---------------------
Output formats
---------------------

-------txt-------
By default, the output polygons are saved in a txt file, which lists the vertices, edges and faces. Each face has an associated label, usually label 0 is reserved for the background.

Line 1 specifies the height and width of the image. 
Line 2 specifis the total number of vertices, the total number of edges, and the total number of faces.

Vertices are listed starting from line 3 in the following format:
vertex_id  x  y

Edges are listed after all vertices have been listed, in the following format:
edge_id  vertex1_id  vertex2_id

Faces are listed after all edges. In particular, a face is a polygon with 0 or more holes, saved in the following format:
                   face_id  number_of_contours  label             \n
(Outer contour)    edge1_id  is_v1_v2   edge2_id  is_v1_v2  ...   \n
(Inner contour 1)  edge1_id  is_v1_v2   edge2_id  is_v1_v2  ...   \n
...                ...       ...        ...       ...       ...

Coordinate system: The center of the bottom-left pixel is (0,0), with x-axis pointing rightward and y-axis pointing upward. 

-------svg-------
Polygons are also saved as an svg file for easy visualization purposes.


---------------------
Read txt output
---------------------
