tool_coor=csvread('tool_coor.csv');
%算出base to tool
tool_pos=tool_coor(1:3);
tool_angle=flip(tool_coor(4:6)*pi/180);
%這個順序為ZYX
T_base_tool_33=eul2rotm(tool_angle);

% theta1=tool_angle(1);
% theta2=tool_angle(2);
% theta3=tool_angle(3);
% 
% z_rotate33=[cos(theta3) -sin(theta3) 0;
%            sin(theta3) cos(theta3) 0;
%            0 0 1];
% x_rotate33=[1 0 0;
%           0  cos(theta1) -sin(theta1);
%           0 sin(theta1) cos(theta1)];
% y_rotate33=[cos(theta2) 0 sin(theta2);
%             0 1 0;
%  
%            -sin(theta2) 0 cos(theta2)];   
% T_base_tool_33=z_rotate33*y_rotate33*x_rotate33

T_base_tool=[T_base_tool_33 tool_pos.'; 0 0 0 1];

T_tool_cam=inv(T_base_tool)*T_base_cam

         