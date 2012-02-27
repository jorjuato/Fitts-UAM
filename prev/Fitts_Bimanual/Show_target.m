function VCODE  = fcn(target, state, target_type, x_index, y_index, num_states, stateindices)
% This block supports an embeddable subset of the MATLAB language.
% See the help menu for details. 

% VCODEs are length 20.
% All dimensions are units of (m).
% VCODE index definition: 1-5 as the same for all target types
% 1 - Target type
%		1=circle
%		2=ellipse
%       3=rectangle
%		4=line
%       5=triangle
%		6 and higher - not implemented yet
% 2 - status
%		0=off
%		1=on
% 3 - x position (m)
% 4 - y position (m)
% 5 - fill colour
% 6 - stroke colour
% 7 - stroke width
% 8 - reserved
% 9 - reserved
%
% For circles
% 10 - radius (m)
%
% For ellipses
% 10 - major radius (m)
% 11 - minor radius (m)
% 12 - orientation of major axis (rad)
%
% For rectangles
% 10 - length (m)
% 11 - height (m)
% 12 - orientation (rad) (relative to x,y already shown)
%
% For lines
% 10 - end point x-axis coordinate relative to start point (m)
% 11 - end point y-axis coordinate relative to start point (m)
%
% For triangles
% 10 - length (m)
% 11 - height (m)
% 12 - offset of peak from midpoint of length (m)
% 13 - orientation (rad)

VCODE=zeros(1,20);		%default - target is not defined

% state<0 or state>num_states is undefined
if (state<0) || (state>num_states)
	return;							%state==0 means that the target is off
end

VCODE(1)=target_type;

% state==0 means that the target is off
if (state==0) 
	VCODE(2)=0;						%target is 'off'
	return;							%state==0 means that the target is off
else
	VCODE(2)=1;						%target is 'on'
end

VCODE(2)=1;							%default is that target is 'on'
VCODE(3)=target(x_index)*0.01;		%target position in target table is cm (v1.4)
VCODE(4)=target(y_index)*0.01;		%target position in target table is cm (v1.4)

% Fill colour currently comes from the GUI in 1.x format: RRRGGGBBB. Convert this to a 32-bit 
% 0xRRGGBBAA value, or use zero for transparency if target table index for the colour is zero.
if stateindices(state,1)==0
	VCODE(5)=0;
else
	VCODE(5)=colourshift(target(stateindices(state,1)));			%fill colour
end

% Stroke colour currently comes from the GUI in 1.x format: RRRGGGBBB. Convert this to a 32-bit 
% 0xRRGGBBAA value, or use zero for transparency if target table index for the colour is zero.
if stateindices(state,2)==0
	VCODE(6)=0;				%if stroke-colour column==0, then stroke colour=0
else
	VCODE(6)=colourshift(target(stateindices(state,2)));			%stroke colour
end

%if target table index for stroke width is zero, then the stroke width is undefined
if stateindices(state,3)==0
	VCODE(7)=0;					%if stroke-width column==0, then stroke width=0
elseif stateindices(state,3)<0
	VCODE(7)=0.001;			%if stroke is not defined, then a 1mm default is chosen
else
	VCODE(7)=target(stateindices(state,3))*0.01;	%stroke width in target table is cm 
end


%circle
if target_type==1
	VCODE(10)=target(stateindices(state,4))*0.01;		%radius (m) -- radius in target table is cm (v1.4)
%ellipse
elseif target_type==2
	VCODE(10)=target(stateindices(state,4))*0.01;		%major radius (m) -- radius in target table is cm (v1.4)	
	VCODE(11)=target(stateindices(state,5))*0.01;		%minor radius (m) -- radius in target table is cm (v1.4)	
	if stateindices(state,6)==0
		VCODE(12)=0;				%if orientation column==0, then orientation=0
	else
		VCODE(12)=mod(target(stateindices(state,6)),360)*pi/180.0;  %orientation (rad) -- orientation in target table is degrees (v1.4)
	end
%rectangle
elseif target_type==3
    VCODE(10)=target(stateindices(state,4))*0.01;       %length (m) -- length in target table is in cm
    VCODE(11)=target(stateindices(state,5))*0.01;       %height (m) -- height in target table is in cm
	if stateindices(state,6)==0
		VCODE(12)=0;				%if orientation column==0, then orientation=0
	else
		VCODE(12)=mod(target(stateindices(state,6)),360)*pi/180.0;  %orientation (rad) -- orientation in target table is degrees (v1.4)
	end
%line
elseif target_type==4
    VCODE(10)=target(stateindices(state,4))*0.01;  % line endpoint in x axis, relative to start (converting to m from cm in target table)
    VCODE(11)=target(stateindices(state,5))*0.01;  % line endpoint in y axis, relative to start (converting to m from cm in target table)
%triangle
elseif target_type==5
    VCODE(10)=target(stateindices(state,4))*0.01;      %length (m) - length in target table is in cm

    if stateindices(state,5)==0
        VCODE(11)=sin(pi/3.0) * VCODE(10);             %height for equilateral triangle
        VCODE(12)=0;                                   %peak offset for equilateral triangle
    else
        VCODE(11)=target(stateindices(state,5))*0.01;  %height (m) - height in target table is in cm
        VCODE(12)=target(stateindices(state,6))*0.01;  %peak offset (m) - offset of peak of triangle, as measured
    end                                                %  relative to midpoint of the length (in m in target table)
    
	if stateindices(state,7)==0
		VCODE(13)=0;				                   %if orientation column==0, then orientation=0
	else
		VCODE(12)=mod(target(stateindices(state,7)),360)*pi/180.0;  %orientation (rad) -- orientation in target table is degrees (v1.4)
	end
end


%colourshift converts an RRRGGGBBB colour into an RRGBBAA, as an uint32 
function RRGGBBAA = colourshift(RRRGGGBBB)
	blue = cast(mod(RRRGGGBBB,1000),'uint32');
	green= cast(mod(RRRGGGBBB-blue,1000000)/1000,'uint32');
	red  = cast((RRRGGGBBB-green*1000-blue)/1000000,'uint32');
    RRGGBBAA =   bitshift(red,24)...		% red component
				+ bitshift(green,16)...		% green component
				+ bitshift(blue,8)...		% blue component
				+ 255;						% alpha component
