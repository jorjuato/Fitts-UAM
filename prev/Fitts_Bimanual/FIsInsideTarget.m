function intarget = FIsInsideTarget(handpos, target_type, attribcol, num_targets, Target_Table)
% all dimensions in m, rad
% arguments:
% handpos: x,y positions of center of target
% attribcol: columns in target table specifying target attributes.  
% Specific to Target_Type
%	circle:
%	[X0,Y0,r...,rn] where:
%	X0,Y0 : centre of circle
%	r : logical radius of circle (i.e. radius used to determine 'in' or 'out' of target)
%	rn: logical radius of other targets (e.g. 2nd, 3rd) also specified in target row.
%   Up to 4 targets can be specified in a row
% ellipse:
%	[X0,Y0,a,b,t...,an, bn, tn] where:
%	X0,Y0 : centre of ellipse
%	a,b : major, minor radii of ellipse
%	t : rotation of ellipse in degrees
%	an,bn : major, minor radii of ellipse of other targets (e.g. 2nd, 3rd) also specified in target row
%	tn : rotation of ellipse in degrees of other targets (e.g. 2nd, 3rd) also specified in target row
%   Up to 4 targets can be specified in a row


target_col_x=1;
target_col_y=2;

x=handpos(1);
y=handpos(2);

intarget = zeros(size(Target_Table,1),4);

X0 = Target_Table(:,target_col_x)*0.01;			%Target Table sizes in cm in v1.4
Y0 = Target_Table(:,target_col_y)*0.01;			%Target Table sizes in cm in v1.4

deltax=x-X0;
deltay=y-Y0;

INT_MAX=2^32-1;

if target_type==1			%circle
	for target=1:num_targets
		r = Target_Table(:,attribcol(target))*0.01;			%Target Table sizes in cm in v1.4
		% simple euclidean distance
		distance2 = deltax.^2 + deltay.^2;					%distance2 is the square of the distance
	    intarget(:,target) = (distance2 <= r.^2);
	end
elseif target_type==2		%ellipse
	for target=1:num_targets
	    a = Target_Table(:,attribcol(target*3-2))*0.01;			%Target Table sizes in cm in v1.4
	    b = Target_Table(:,attribcol(target*3-1))*0.01;			%Target Table sizes in cm in v1.4	
	    t = Target_Table(:,attribcol(target*3))*(pi / 180.0);	%Target Table angles in degrees in v1.4
		sint=sin(t);
		cost=cos(t);
		xrotate=deltax.*cost+deltay.*sint;		%location of x point in ellipse's coordinate frame
		yrotate=-deltax.*sint+deltay.*cost;		%location of y point in ellipse's coordinate frame
		m=yrotate./xrotate;						%slope of line between the (x,y) to (x0,y0) in ellipse's coordinate frame
		y2sq=1./(1./(m.*a).^2+1./b.^2);			%y2sq is y2^2, where y2 is the intersection of the ellipse and the (x,y) to (x0,y0) line
	    intarget(:,target) = (yrotate.^2 <= y2sq);
    end
elseif target_type==3       %rectangle
    for target=1:num_targets
        w = Target_Table(:,attribcol(target*3-2))*0.01;  %width (sizes in cm in v1.4)
        h = Target_Table(:,attribcol(target*3-1))*0.01;  %height (sizes in cm in v1.4)
        
        if attribcol(target*3) > 0
            t = Target_Table(:,attribcol(target*3))*(pi / 180.0);  %angle (angles in degrees in v1.4)
        else
            t = zeros(size(Target_Table, 1), 1);
        end

        sint=sin(t);
        cost=cos(t);

        xrotate=deltax.*cost + deltay.*sint;   %location of x point in rectangle's coordinate frame
        yrotate=-deltax.*sint + deltay.*cost;  %location of y point in rectangle's coordinate frame

        intarget(:,target) = abs(xrotate) <= w/2 & abs(yrotate) <= h/2;
    end
elseif target_type==4       %line
    for target=1:num_targets
        % treat the line like a rectangle of height h and width w
        % determine height (stroke width) in the same fashion as in Show_Target
        if attribcol(target*3-2) == 0                     %0 column index means stroke width of 0
            h = zeros(size(Target_Table, 1), 1);
        elseif attribcol(target*3-2) < 0                  %-ve column index means stroke width of 1mm
            h = ones(size(Target_Table, 1), 1) .* 0.001;
        else
            h = Target_Table(:,attribcol(target*3-2)) * 0.01;  %target table contains width in cm
        end
        
        ex = Target_Table(:,attribcol(target*3-1))*0.01;  %endpoint in x relative to start point (cm)
        ey = Target_Table(:,attribcol(target*3))*0.01;    %endpoint in y relative to start point (cm)
        
        w = sqrt(ex.^2 + ey.^2);                          %width (cm)
        
        % determine the vector from the line's midpoint to the hand position
        handx = deltax - ex/2;
        handy = deltay - ey/2;
        
        t = atan2(ey, ex);                                %calculate angle of the line (rads)
        sint = sin(t);
        cost = cos(t);
        
        xrotate = handx.*cost + handy.*sint;   %location of x point in rectangle's coordinate frame
        yrotate = -handx.*sint + handy.*cost;  %location of y point in rectangle's coordinate frame
 
        intarget(:,target) = abs(xrotate) <= w/2 & abs(yrotate) <= h/2;
    end
elseif target_type==5  % triangle
    for target=1:num_targets
        w = Target_Table(:,attribcol(target*4-3))*0.01;         %length (sizes in cm in v1.4)
        
        % If a height column of 0 is specified, make the triangle equilateral. Otherwise, use the
        % values for height and peak offset from the target table.
        if attribcol(target*4-2) > 0
            h = Target_Table(:,attribcol(target*4-2))*0.01;  %height (sizes in cm in v1.4)
            p = Target_Table(:,attribcol(target*4-1))*0.01;  %peak offset (sizes in cm in v1.4)
        else
            h = w.*sin(pi/3.0);
            p = zeros(size(Target_Table, 1), 1);
        end
        
        if attribcol(target*4) > 0
            t = Target_Table(:,attribcol(target*4))*(pi / 180.0);  %angle (angles in degrees in v1.4)
        else
            t = zeros(size(Target_Table, 1), 1);
        end
        
        sint = sin(t);
        cost = cos(t);
        
        xrotate = deltax.*cost + deltay.*sint;   %location of x point in triangle's coordinate frame
        yrotate = -deltax.*sint + deltay.*cost;  %location of y point in triangle's coordinate frame
        
        % Begin to find centroid of triangle relative to the point on the base beneath the peak. "Left vertex" 
        % refers to the vertex at the left end of the base, and "right vertex" to the vertex at the right end.
        n = w./2.0 + p;  % distance along base from left vertex to peak
        m = w./2.0 - p;  % distance along base from peak to right vertex

        alpha = atan2(h, n);  % angle between base and line from left vertex to peak
        beta = atan2(h, m);   % angle between base and line from right vertex to peak
        
        length1 = sqrt(h.*h + n.*n) ./ 2.0;  % half length of line from left vertex to peak
        length2 = sqrt(h.*h + m.*m) ./ 2.0;  % half length of line from right vertex to peak

        % Determine slope of line from left vertex to midpoint of line from right vertex to peak.
        slope1 = ones(size(Target_Table, 1), 1) .* INT_MAX;
        
        for i=1:size(Target_Table, 1)
            if abs(w(i) - length2(i)*cos(beta(i))) > 0.0
                slope1(i) = length2(i)*sin(beta(i)) / (w(i) - length2(i)*cos(beta(i)));
            end
        end

        % Determine slope of line from right vertex to midpoint of line from left vertex to peak.
        slope2 = ones(size(Target_Table, 1), 1) .* INT_MAX;

        for i=1:size(Target_Table, 1)
            if abs(w(i) - length1(i)*cos(alpha(i))) > 0.0
                slope2(i) = -length1(i)*sin(alpha(i)) / (w(i) - length1(i)*cos(alpha(i)));
            end
        end

        % Calculate centroid position.
        centroid_x = -(m.*slope2 + n.*slope1) ./ (slope1 - slope2);
        centroid_y = slope1.*(centroid_x + n);

        % Test if the point is under the left side of the peak.
        is_under_left = false(size(Target_Table, 1), 1);

        for i=1:size(Target_Table, 1)
            if n(i) >= 0
                is_under_left(i) = (h(i) + (h(i)/n(i)) * (xrotate(i) + centroid_x(i))) >= (yrotate(i) + centroid_y(i));
            else
                is_under_left(i) = (h(i) + (h(i)/n(i)) * (xrotate(i) + centroid_x(i))) <= (yrotate(i) + centroid_y(i));
            end
        end
        
        % Test if the point is under the right side of the peak.
        is_under_right = false(size(Target_Table, 1), 1);

        for i=1:size(Target_Table, 1)
            if m(i) >= 0
                is_under_right(i) = (h(i) - (h(i)/m(i)) * (xrotate(i) + centroid_x(i))) >= (yrotate(i) + centroid_y(i));
            else
                is_under_right(i) = (h(i) - (h(i)/m(i)) * (xrotate(i) + centroid_x(i))) <= (yrotate(i) + centroid_y(i));
            end
        end
        
        intarget(:,target) = is_under_right & ...            %under right side of peak
                             is_under_left & ...             %under left side of peak
                             (yrotate + centroid_y) >= 0.0;  %above base
    end
end


