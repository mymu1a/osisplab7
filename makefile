build: parent

parent: main.cc circleQueue.cc produser.cc consumer.cc doctorQueue.cc
	gcc -o parent $^
