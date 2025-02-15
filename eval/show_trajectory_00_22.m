clc
clear all
close all

load('2019-11-29-09-00-22.bag_commands_log.mat')
load('2019-11-29-09-00-22.bag_collision_points_log.mat')

i_start = 1341;
i_end = i_start + 20;

% last clip
 i_start = find(time>= 382, 1);
 i_end = find(time>= 402, 1);
% first clip
% i_start = find(time>= 202, 1);
% i_end = find(time>= 216, 1);
% second clip
%i_start = find(time>= 262, 1);
%i_end = find(time>= 282, 1);

if false

position = zeros(2,i_end-i_start+2);
velocities = zeros(2,i_end-i_start+2);
nominal_velocities_p_ref = zeros(2,i_end-i_start+2);
angle = zeros(2,i_end-i_start+2);
for i = 2:(i_end-i_start+2)
    position(:,i) = position(:,i-1) + (time(i_start+i-1)-time(i_start+i-2))*...
       commands(4, i_start+i-2)*[cos(angle(i-1)); sin(angle(i-1))];
    velocities(:,i-1) = commands(4, i_start+i-2)*[cos(angle(i-1)); sin(angle(i-1))];
    nominal_velocities_p_ref(:,i-1) = commands(2, i_start+i-2)*[cos(angle(i-1)); sin(angle(i-1))] + ...
        commands(3, i_start+i-2)*0.4*[-sin(angle(i-1)); cos(angle(i-1))];
    angle(i) = angle(i-1) + (time(i_start+i-1)-time(i_start+i-2))*commands(5, i_start+i-2);
end

plot(position(1,:), position(2,:))

hold on

j = 1;
k = 1;
for i = i_start:i_end
    tf_collision_pts = (ones(length(collision_points{i}(1,:)),1)*position(:,k)')' + ...
        [cos(angle(k)-pi/2), -sin(angle(k)-pi/2);sin(angle(k)-pi/2),cos(angle(k)-pi/2)]*collision_points{i}(1:2,:);
     plot(tf_collision_pts(1,:), tf_collision_pts(2,:), 'r.', 'MarkerSize',j+10)
     plot(tf_collision_pts(1,:), tf_collision_pts(2,:), 'go', 'MarkerSize',2)
     plot(position(1,k), position(2,k), 'ko', 'MarkerSize',5)
     plot([position(1,k), position(1,k)+nominal_velocities_p_ref(1,k)], [position(2,k), position(2,k)+nominal_velocities_p_ref(2,k)])
     xlim([-4,8])
     ylim([-8,4])
     daspect([1 1 1])
     pause(0.25)
     j = j*1.2;
     k = k + 1;
end

end

normal_statistics = [];
tangent_statistics = [];
distance_sequence = zeros(1,1-i_start+i_end);
for i = i_start:i_end
    ref_point_y = 0.25;
    v_ref_point = [-ref_point_y*commands(5,i); commands(4,i)];
    d_min = nan;
    for j = 1:size(collision_points{i}, 2)
        d_vec = collision_points{i}(:, j) - [0; ref_point_y];
        d = sqrt(d_vec'*d_vec);
        if d > 0
            normal_statistics = [normal_statistics; [d, d_vec'*v_ref_point/d]];
            tangent_statistics = [tangent_statistics; [d, [d_vec(2);-d_vec(1)]'*v_ref_point/d]];
        end
        if ~(d > d_min)
            d_min = d;
        end
        distance_sequence(i-i_start+1) = d_min;
    end
end
figure
hold on
plot(normal_statistics(:,1), normal_statistics(:,2), 'ko')
plot(tangent_statistics(:,1), tangent_statistics(:,2), 'ro')
figure
% plot(time(i_start:i_end), distance_sequence)
% xlabel('time [s]')
% ylabel('shortest distance [m]')
plot(time(i_start:i_end)-time(i_start), distance_sequence)
ylim([0,2.6])
xlabel('time [s]')
ylabel('normalized shortest distance [m/s]')



%% Reading Log files
    clc
    nfig = 2;
    clear command_U command_R 
    Omega_max = 4.124 /4;
    Vel_max = 1;
       % Case 1: No Crowd
    idt1 = find(time>= 202, 1);
    idt2 = find(time>= 216, 1);
        % Case 2: 1D flow unidirectional
%     idt1 = find(time>= 262, 1);
%     idt2 = find(time>= 282, 1);
        % Case 3: 1D flow bidirectional
%     idt1 = find(time>= 382, 1);
%     idt2 = find(time>= 402, 1);

    trange = idt1:idt2;    
%     User input Commnads
    Command_U = commands(2:3,trange);
%     Robot Commnads
    Command_R = commands(4:5,trange);
    
    [linear_diff,heading,disagreement ,Contribution] = similarity(Command_U ,Command_R, Vel_max, Omega_max);
    [fluency] = user_fluency(Command_U, Vel_max, Omega_max)
    Contribution_score = [mean(Contribution); std(Contribution)]
    linear_diff
    heading
    disagreement
    fluency
%%
    figure(nfig)
    subplot(2,1,1),plot(time(trange)-time(idt1),Command_U(1,:));
    hold on;
    grid on;
    subplot(2,1,1),plot(time(trange)-time(idt1),Command_R(1,:))
    title('Linear Speed')
    
    subplot(2,1,2),plot(time(trange)-time(idt1),Command_U(2,:));
    hold on;
    grid on;
    subplot(2,1,2),plot(time(trange)-time(idt1),Command_R(2,:))
    title('Angular Speed')
