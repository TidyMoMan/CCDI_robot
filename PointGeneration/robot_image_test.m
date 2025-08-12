windowSize = 5;

img = imread("C:\Users\moses\Downloads\CCDI.png");
img = imresize(img, 1);
img = rgb2gray(img);

img = (img(:, :) >= 90 );

%kernel = ones(windowSize) / windowSize ^ 2;
%img = imfilter(img, kernel);

img = (img(:, :) >= 1 );

img = imcrop(img, [0, 0, 120, 90]);


%img = imread("C:\Users\moses\Downloads\frame.png");


imshow(img)

%find a black point
    %then go until a white point is seen
        %write that out as one ray
    %repeat until the end of the row
%repeat for every row



rayStarted = 0;
rayIdx = 1;

%x , y, isPenUp (for next ray)?
rayList = zeros(3, 1);
img = ~img;

for y =  1:height(img)

    rayStarted = 0; %reset every row

    for x = 1:width(img)

        fprintf("testing pixel (%f, %f); value is %f\n", y, x, img(y, x))

        %ray is starting
        if img(y, x) == 1 & rayStarted == 0
            rayStarted = 1;
            rayList(1, rayIdx) = x;
            rayList(2, rayIdx) = y;
            rayList(3, rayIdx) = 1; %move to start position with pen up
            rayIdx = rayIdx + 1;

            rayList(1, rayIdx) = x;
            rayList(2, rayIdx) = y;
            rayList(3, rayIdx) = 0; %then bring pen down
            rayIdx = rayIdx + 1;
        end

        %ray is ending
        if img(y, x) == 0 & rayStarted == 1
            rayStarted = 0;
            rayList(1, rayIdx) = x;
            rayList(2, rayIdx) = y;
            rayList(3, rayIdx) = 0; %finish drawing ray with pen down
            rayIdx = rayIdx + 1;

            rayList(1, rayIdx) = x;
            rayList(2, rayIdx) = y;
            rayList(3, rayIdx) = 1; %then bring pen up
            rayIdx = rayIdx + 1;
        end

    end
    rayList(1, rayIdx) = x;
    rayList(2, rayIdx) = y;
    rayList(3, rayIdx) = 1; %then bring pen up
    rayIdx = rayIdx + 1;
end

writematrix(rayList, "raylist.csv");







