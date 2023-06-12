#Gurjas Bassi 400062217
#This portion was done in VScode and it worked fine whereas in IDLE it did not work due to some issues i was unable to debug 
#using Python 3.10 this also worked i believe 

import numpy as np
import open3d as o3d
if __name__ == "__main__":
    #Remember the goals of modularization
    #   -- smaller problems, reuse, validation, debugging
    #To simulate the data from the sensor lets create a new file with test data 
   
    #Read the test data in from the file we created        
    print("Read in the prism point cloud data (pcd)")
    pcd = o3d.io.read_point_cloud("demo.xyz", format="xyz")  #reads xyz that is created from the python script 

    #Lets see what our point cloud data looks like numerically       
    print("The PCD array:")
    print(np.asarray(pcd.points))

    #Lets see what our point cloud data looks like graphically       
    print("Lets visualize the PCD: (spawns seperate interactive window)")
    o3d.visualization.draw_geometries([pcd])

    #OK, good, but not great, lets add some lines to connect the vertices
    #   For creating a lineset we will need to tell the packahe which vertices need connected
    #   Remember each vertex actually contains one x,y,z coordinate

    #Give each vertex a unique number
    yz_slice_vertex = []
    for x in range(0,1536):
        yz_slice_vertex.append([x])

    #Define coordinates to connect lines in each yz slice        
    lines = []  
    for x in range(0,1536,4):
        lines.append([yz_slice_vertex[x], yz_slice_vertex[x+1]])
        lines.append([yz_slice_vertex[x+1], yz_slice_vertex[x+2]])
        lines.append([yz_slice_vertex[x+2], yz_slice_vertex[x+3]])
        lines.append([yz_slice_vertex[x+3], yz_slice_vertex[x]])

    #Define coordinates to connect lines between current and next yz slice        
    for x in range(0,1500,4):
        lines.append([yz_slice_vertex[x], yz_slice_vertex[x+4]])
        lines.append([yz_slice_vertex[x+1], yz_slice_vertex[x+5]])
        lines.append([yz_slice_vertex[x+2], yz_slice_vertex[x+6]])
        lines.append([yz_slice_vertex[x+3], yz_slice_vertex[x+7]])

    #This line maps the lines to the 3d coordinate vertices
    line_set = o3d.geometry.LineSet(points=o3d.utility.Vector3dVector(np.asarray(pcd.points)),lines=o3d.utility.Vector2iVector(lines))

    #Lets see what our point cloud data with lines looks like graphically       
    o3d.visualization.draw_geometries([line_set])  #this is the same example as in week 9 lecture 
                                    
    
 
