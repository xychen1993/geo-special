%read files
folder = '/Users/falcon/Downloads/sample_drive/test/';
files = dir(folder);

%abstract feature
add = uint8(zeros(2032, 2032, 1));
pre = rgb2gray(imread(strcat(folder, files(4).name)));
for i = 5: 20 : length(files) 
    img = rgb2gray(imread(strcat(folder, files(i).name)));
    sub = imsubtract(img,pre);
    add = imadd(add ,sub);
    pre = img;
end
%edge detection
img2 = rgb2gray(imread(strcat(folder, files(5).name)));
[junk,threshold] = edge(img2, 'sobel'); 
Edges = imfill(imdilate(edge(img2,'sobel', threshold * 0.2), [strel('line', 12.5, 90) strel('line', 12.5, 0)],2), 'holes');
imgEdges = imerode(imcomplement(Edges),strel('diamond',1));
imgEdges = imerode(imgEdges,strel('diamond',1));

%binarize,erode and add edges 
binary = im2bw(imcomplement(add), 0.8);
erode = imerode(binary, strel('disk', 8));
mask= imadd(erode,imgEdges);

%save mask 
imwrite(imresize(mask, 0.75), 'mask.jpg'); 
imwrite(imresize(add, 0.75), 'add.jpg'); 



