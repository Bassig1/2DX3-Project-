

#Modify the following line with your own serial port details
#   Currently set COM3 as serial port at 115.2kbps 8N1
#   Refer to PySerial API for other options.  One option to consider is
#   the "timeout" - allowing the program to proceed if after a defined
#   timeout period.  The default = 0, which means wait forever.
#Gurjas Bassi 400062217
#This portion was done in VScode and it worked fine whereas in IDLE it did not work due to some issues i was unable to debug 
# This keeps going for the demo until enter is pressed on the keyboard 
import serial
import math
import re  # this potentially could have been used if the string contained letters to not require this the python script must be started after the mirocontroller is reset
import keyboard # this is used to stop the code when enter is pressed it is for the demo during the lab 
s = serial.Serial('COM7',115200, timeout = 10) # this was changed to COM7 due to this being the UART Port that interacts with the mirco 
print("Opening: " + s.name)
# reset the buffers of the UART port to delete the remaining data in the buffers
s.reset_output_buffer()
s.reset_input_buffer()

x = 0 # for the demo there is no displacement value so this stays at 0
f = open("demo.xyz", "w")# this allows for writing of the .xyz file that will be used for visualization
i = 0 #iterate value for the while loop
while True: #runs until the enter button is pressed on the keyboard via the ide which is used in this case VSCode is used, this while true came from the first refrnece to keep the loop going until the keyboard button is pressed in this case enter 
    a = s.readline() #a is to be used due to x being used for xyz in this script 
    t = a.decode() #just like in studio this decodes the string to give the number distance value or whatever the UART is sending
    if i>512: #this is due to 512 being 360 degrees 
        i=0  # if i > 512 it resets to 0 degrees for the angle     
    y = int(t)*math.cos((2*math.pi)*((i)/512))  #the y value is calucated like in lab with it being the cos(angle) with angle being 2*pi*(i/511) with 511 being 360 in this case
    z = int(t)*math.sin((2*math.pi)*((i)/512)) # the same principle can be applied for the z axis but in this case it is sin now instead
    i += 1  #increases up to 512 for 360 degrees # thisis after the first result due it being 0 degrees for accurate visualization
    f.write("{} {} {}\n".format(x, y, z))  # this is used for writing array 3 columns refrence link is : https://stackoverflow.com/questions/47078585/python-f-write-is-not-taking-more-arguments
    if keyboard.is_pressed('enter'):  # this is to ensure the file is closed for writing enter must be pressed in terminal to exit this loop link:https://www.pythonforbeginners.com/basics/how-to-detect-keypress-in-python
        f.close() #this closes the file that is being ued 
        break  # if the enter button is pressed break occurs and file is closed as well only way to stop the python script 
    print(t)  # this is just for the python terminal to show the distance that is being showed similar to realterm in labs and studios mainly for debugging purposes this prints the distance number
#Gurjas Bassi 400062217
#This portion was done in VScode and it worked fine whereas in IDLE it did not work due to some issues i was unable to debug 
#This is the visualization which is done in the same script for ease of use however to just see the visualization with the .xyz file another pair of scripts has been provided 
import numpy as np
import open3d as o3d
if __name__ == "__main__":
    #Remember the goals of modularization
    #   -- smaller problems, reuse, validation, debugging
    #To simulate the data from the sensor lets create a new file with test data 
   
    #Read the test data in from the file we created        
    print("Read in the prism point cloud data (pcd)")
    pcd = o3d.io.read_point_cloud("demo.xyz", format="xyz")

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
    o3d.visualization.draw_geometries([line_set])
                                    
    
 
#refrences are the following links
#https://makersportal.com/blog/2018/2/25/python-datalogger-reading-the-serial-output-from-arduino-to-analyze-data-using-pyserial
#https://www.tutorialspoint.com/python/string_decode.htm
#https://www.tutorialspoint.com/python/string_decode.htm
#https://stackoverflow.com/questions/1249388/removing-all-non-numeric-characters-from-string-in-python
#https://stackoverflow.com/questions/47078585/python-f-write-is-not-taking-more-arguments
#https://www.pythonforbeginners.com/basics/how-to-detect-keypress-in-python