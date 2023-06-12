

#Modify the following line with your own serial port details
#   Currently set COM3 as serial port at 115.2kbps 8N1
#   Refer to PySerial API for other options.  One option to consider is
#   the "timeout" - allowing the program to proceed if after a defined
#   timeout period.  The default = 0, which means wait forever.
#Gurjas Bassi 400062217
#This portion was done in VScode and it worked fine whereas in IDLE it did not work due to some issues i was unable to debug 
# This keeps going for the demo until enter is pressed on the keyboard 
import serial #as was shown in studio 9
import math
import re  # this potentially could have been used if the string contained letters to not require this the python script must be started after the mirocontroller is reset refrence link:https://stackoverflow.com/questions/1249388/removing-all-non-numeric-characters-from-string-in-python
import keyboard # this is used to stop the code when enter is pressed 
s = serial.Serial('COM7',115200, timeout = 10) # this was changed to COM7 due to this being the UART Port that interacts with the mirco this is default such as for studio 9
print("Opening: " + s.name)
# reset the buffers of the UART port to delete the remaining data in the buffers
s.reset_output_buffer()
s.reset_input_buffer()

x = 0 # this is the displacement value which when moving the combo will need to change as well for example this will change by 1 m once halfway of the for loop is done iterating
f = open("data.xyz", "w")# this allows for writing of the .xyz file that will be used for visualization
i = 0 #variaible used to iterate and to calucate the angle at which the motors at when i = 512 its at 360 degrees which is 2pi
while True: #runs until the enter button is pressed on the keyboard via the ide which is used in this case VSCode is used initally for loop was used but for project case recording the xyz has to occur until user wants it to stop for hallway specifically, this while true came from the first refrnece to keep the loop going until the keyboard button is pressed in this case enter 
    a = s.readline() #a is to be used due to x being used for xyz in this script 
    t = a.decode() #just like in studio this decodes the string to give the number distance value or whatever the UART is sending 
    if i>512: #this is due to 512 being 360 degrees 
        i=0  # if i > 512 it resets to 0 degrees for the angle 
        x = x+1000 # this changes the displacmeent value by 1 m for each time the motor is done rotating due to the length of the hallway that is to be captured displacement is 1m for the file that was made the hallway was displaced 2 m since it is the same throughout and due to the strength of the appartus holding the ToF it was unable to capture thw whole hallway   
    y = int(t)*math.cos((2*math.pi)*((i)/512))  #the y value is calucated like in lab with it being the cos(angle) with angle being 2*pi*(i/511) with 511 being 360 in this case
    z = int(t)*math.sin((2*math.pi)*((i)/512)) # the same principle can be applied for the z axis but in this case it is sin now instead
    i += 1
    f.write("{} {} {}\n".format(x, y, z)) # this is used for writing array 3 columns refrence link is : https://stackoverflow.com/questions/47078585/python-f-write-is-not-taking-more-arguments
    if keyboard.is_pressed('enter'):  # this is to ensure the file is closed for writing link: https://www.pythonforbeginners.com/basics/how-to-detect-keypress-in-python
        f.close() #this closes the file that is being ued 
        #after the enter button is pressed user should stop uart and motor spinning with push buttons respectivley
        break  # if the enter button is pressed break occurs and file is closed as well only way to stop the python script, when this is pressed code goes to the import numpy part so visulizatoin can take place 
    print(t)  # this is just for the python terminal to show the distance that is being showed similar to realterm in labs and studios
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
    pcd = o3d.io.read_point_cloud("data.xyz", format="xyz")

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
    for x in range(0,2048):
        yz_slice_vertex.append([x])

    #Define coordinates to connect lines in each yz slice        
    lines = []  
    for x in range(0,2048,4):
        lines.append([yz_slice_vertex[x], yz_slice_vertex[x+1]])
        lines.append([yz_slice_vertex[x+1], yz_slice_vertex[x+2]])
        lines.append([yz_slice_vertex[x+2], yz_slice_vertex[x+3]])
        lines.append([yz_slice_vertex[x+3], yz_slice_vertex[x]])

    #Define coordinates to connect lines between current and next yz slice        
    for x in range(0,2040,4):
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