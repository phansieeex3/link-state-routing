import time

print("Running Dikstra's...")
time.sleep(3)
file_name = "./network_data.txt"

print("Reading in file")
f = open(file_name)

f.close
print("Writing to file")
f = open(file_name, 'w')
f.write("2:3\n")
f.write("3:3\n")
f.write("4:3")
f.close()
print("Done!")
