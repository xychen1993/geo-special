function []=main()
format long;
I=AVGI();
c=1;
for i=0:1:2031
    for j=0:1:2031
    x(2032*i+j+1)=-1+1/1015.5*j;
    y(2032*i+j+1)=-1+1/1015.5*i;
    z(2032*i+j+1)=I(i+1,j+1);
    end
end
for i=1:1:2032
    for j=1:1:2032
        countx(c)=i;
        county(c)=j;
        c=c+1;
    end
end
c=c-1;
for i=1:c
    med(i)=I(countx(i),county(i));
end
m1=median(med);
m2=min(med);
m3=max(med);
p=1;
for i=1:c
        if I(countx(i),county(i))<(m3-m1)/3+m1
            hx(p)=countx(i);
            hy(p)=county(i);
            p=p+1;
        end
end
p=p-1;
o=0.15;
for w=0:10
[countx,county,c]=leastsqure2(countx,county,c,I,x,y,o);
[countx,county,c]=leastsqure2(countx,county,c,I,x,y,o);
draw(countx,county,c,I);
saveas(gcf, strcat(num2str(w),'output'), 'bmp');
o=o-0.01;
end
end





function [a]=AVGI()
p=1;
 for i=393408606:393408610
     if exist(strcat('/Users/XSJ/Google/course/495/sample_drive/cam_0/',num2str(i),'.jpg'),'file') 
      imageName=strcat('/Users/XSJ/Google/course/495/sample_drive/cam_0/',num2str(i),'.jpg');
      m{p}=im2double(rgb2gray(imread(imageName)));
      p=p+1
     end
 end
 p=p-1;
a=zeros(2032,2032);
a=a+m{3};
a=a+m{4};
a=a/2;
 
end

function[]= draw(countx,county,c,I)
for i=1:2032
    for j=1:2032
        d(i,j)=1;
    end
end

for i=2:c
    d(countx(i),county(i))=0;
end
imshow(d);
end

function k=fitting2(I,countx,county,c)
for i=1:c
    x(i)=-1+1/1015.5*(countx(i));
    y(i)=-1+1/1015.5*(county(i));
    z(i)=I(countx(i),county(i));
end
f=@(a,x)a(1)+a(2).*x(1,:)+a(3).*x(2,:)+a(4).*x(1,:).*x(2,:)+a(5).*x(1,:).^2+a(6).*x(2,:).^2;%+a(7).*x(1,:).^2.*x(2,:)+a(8).*x(1,:).*x(2,:).^2+a(9).*x(1,:).^2.*x(2,:).^2;
k=nlinfit([x;y],z,f,zeros(6));

end

function [lx,ly,q]=leastsqure2(countx,county,c,I,x,y,o)
q=1;
k=fitting2(I,countx,county,c);
for i=0:2031
    for j=0:2031
        I0(i+1,j+1)=k(1)+k(2)*x(2032*i+j+1)+k(3)*y(2032*i+j+1)+k(4)*x(2032*i+j+1)*y(2032*i+j+1)+k(5)*x(2032*i+j+1)^2+k(6)*y(2032*i+j+1)^2+k(7)*x(2032*i+j+1)^2*y(2032*i+j+1)+k(8)*x(2032*i+j+1)*y(2032*i+j+1)^2+k(9)*x(2032*i+j+1)^3+k(10)*y(2032*i+j+1)^3+k(11)*x(2032*i+j+1)^3*y(2032*i+j+1)+k(12)*x(2032*i+j+1)*y(2032*i+j+1)^3+k(13)*x(2032*i+j+1)^2*y(2032*i+j+1)^2+k(14)*x(2032*i+j+1)^3*y(2032*i+j+1)^2+k(15)*x(2032*i+j+1)^2*y(2032*i+j+1)^3+k(16)*x(2032*i+j+1)^3*y(2032*i+j+1)^3;
    end
end
for i=1:c
        if abs(I0(countx(i),county(i))-I(countx(i),county(i)))<I(countx(i),county(i))*o
            lx(q)=countx(i);
            ly(q)=county(i);
            q=q+1;
         end

end
q=q-1;

end