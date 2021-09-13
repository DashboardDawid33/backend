sudo docker build -t website/website:latest .
sudo docker run -it -p localhost:8888:8888 website/website