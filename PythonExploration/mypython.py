#import packages
import random
import string

#fucntion for creating and writing to files
def creatFile(filename):
    #open file with filename param
    f = open(filename, "w+")
    #print 10 random lowercase characters and write them to file
    for x in range(10):
        ran = random.choice(string.ascii_lowercase)
        f.write(ran)
        #print without newline character
        print(ran, end="")
    #write a newlline character to file and print a newline
    f.write("\n")
    print()
    #close file
    f.close()

#create 3 files with best dbz character names
creatFile("goku")
creatFile("piccolo")
creatFile("vegeta")

#print random integers within the range of 1-42
num1 = random.randint(1,42)
num2 = random.randint(1,42)
print(num1)
print(num2)
#print product of 2 random numbers
print(num1*num2)
