% Capacities (X-axis)
capacities = [0.05, 0.5, 1, 5, 10];

% Execution Time in Seconds for meta_reag (Y-axis)
%time_opt  = [5.07,  7.98,  9.82, 12.42, 13.20];
%time_arc  = [4.74, 10.37, 11.19, 10.80, 11.94];
%time_2q   = [4.90,  9.90, 10.68,  9.85, 10.58];
%time_lirs = [5.82, 11.02, 12.23, 12.69, 13.51];
%time_lru  = [1.79,  1.84,  1.79,  1.83,  1.52];
%time_lfuk = [5.06,  7.42,  8.08,  8.87, 10.53];
%time_lfu  = [5.79,  8.19,  8.66,  9.07, 10.20];
%time_lrfu = [3.98,  5.48,  6.25,  7.47,  9.29];

% Execution Time in Seconds for meta_rnha (Y-axis)
%time_opt  = [18.65, 30.93, 35.19, 46.21, 49.89];
%time_arc  = [21.16, 27.76, 28.86, 35.55, 41.88];
%time_2q   = [20.27, 25.85, 26.88, 31.60, 33.40];
%time_lirs = [25.03, 30.93, 32.81, 41.65, 47.96];
%5time_lru  = [ 4.10,  4.23,  4.11,  4.15,  3.97];
%time_lfuk = [18.14, 25.40, 26.75, 28.22, 28.40];
%time_lfu  = [20.62, 25.89, 27.01, 32.23, 32.81];
%time_lrfu = [13.05, 17.67, 17.93, 24.66, 30.29];

% Execution Time in Seconds for meta_rprn (Y-axis)
time_opt  = [13.97, 25.75, 29.37, 37.59, 39.92];
time_arc  = [16.96, 23.31, 24.84, 30.03, 32.90];
time_2q   = [16.87, 22.42, 23.58, 25.97, 29.04];
time_lirs = [20.12, 27.51, 28.32, 34.84, 39.01];
time_lru  = [ 4.05,  3.57,  3.47,  3.61,  3.56];
time_lfuk = [14.32, 21.36, 23.03, 25.77, 26.67];
time_lfu  = [16.95, 21.34, 23.07, 25.78, 29.16];
time_lrfu = [ 8.88, 16.73, 15.43, 23.60, 28.42];

figure;
hold on; grid on;

semilogx(capacities, time_opt,  '-k^', 'LineWidth', 2, 'MarkerSize', 8); 
semilogx(capacities, time_arc,  '-ro', 'LineWidth', 2, 'MarkerSize', 8); 
semilogx(capacities, time_2q,   '-cx', 'LineWidth', 2, 'MarkerSize', 8); 
semilogx(capacities, time_lirs, '-bs', 'LineWidth', 2, 'MarkerSize', 8); 
semilogx(capacities, time_lru,  '-md', 'LineWidth', 2, 'MarkerSize', 8); 
semilogx(capacities, time_lfuk, '-yv', 'LineWidth', 2, 'MarkerSize', 8); 
semilogx(capacities, time_lfu,  '-g*', 'LineWidth', 2, 'MarkerSize', 8); 
semilogx(capacities, time_lrfu, '-p',  'Color', [1 0.5 0], 'LineWidth', 2, 'MarkerSize', 8);

set(gca, 'XScale', 'log');
xticks(capacities);
xticklabels({'0.05%', '0.5%', '1%', '5%', '10%'});

xlabel('Cache Capacity (%)', 'FontSize', 12, 'FontWeight', 'bold');
ylabel('Total Execution Time (Seconds)', 'FontSize', 12, 'FontWeight', 'bold');
title('Execution Time vs Cache Capacity (meta\_rprn)', 'FontSize', 14);


legend('OPT', 'ARC', '2Q', 'LIRS', 'LRU', 'LFU-k', 'LFU', 'LRFU', ...
    'Location', 'northwest', 'FontSize', 11);

hold off;