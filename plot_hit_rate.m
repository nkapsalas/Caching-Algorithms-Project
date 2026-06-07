% Capity Percentages 
capacities = [0.05, 0.5, 1, 5, 10];

% Hit Rates for meta_reag (Y-axis)
%hit_rate_opt  = [61.50, 66.57, 68.17, 72.13, 73.12];
%hit_rate_arc  = [55.88, 60.73, 62.22, 65.96, 68.04];
%hit_rate_2q   = [56.52, 60.81, 62.43, 65.79, 66.91];
%hit_rate_lirs = [54.87, 59.79, 61.16, 65.12, 67.23];
%hit_rate_lru  = [55.02, 60.12, 61.68, 65.57, 67.39];
%hit_rate_lfuk = [43.83, 46.68, 47.75, 51.76, 55.13];
%hit_rate_lfu  = [36.58, 42.37, 45.38, 53.52, 57.59];
%hit_rate_lrfu = [35.35, 41.39, 43.22, 49.38, 53.39];

% Hit Rates for meta_rnha (Y-axis)
%hit_rate_opt  = [39.93, 48.18, 51.17, 59.25, 62.42];
%hit_rate_arc  = [32.50, 38.63, 41.14, 48.03, 52.31];
%hit_rate_2q   = [32.77, 38.90, 41.43, 48.42, 50.95];
%hit_rate_lirs = [30.47, 37.51, 39.89, 47.43, 51.81];
%hit_rate_lru  = [31.71, 37.48, 39.99, 46.88, 50.68];
%hit_rate_lfuk = [14.88, 18.07, 19.76, 27.20, 32.82];
%hit_rate_lfu  = [ 9.99, 16.76, 20.25, 30.85, 38.73];
%hit_rate_lrfu = [ 9.78, 14.40, 16.00, 24.69, 30.91];

% Hit Rates for meta_rprn (Y-axis)
hit_rate_opt  = [46.94, 54.15, 56.68, 63.55, 66.16];
hit_rate_arc  = [40.15, 46.26, 48.40, 54.12, 57.50];
hit_rate_2q   = [40.67, 46.48, 48.57, 53.77, 55.97];
hit_rate_lirs = [38.30, 45.52, 47.64, 53.58, 57.26];
hit_rate_lru  = [38.86, 44.83, 47.00, 52.89, 56.00];
hit_rate_lfuk = [22.05, 25.15, 26.77, 33.02, 37.49];
hit_rate_lfu  = [17.93, 23.72, 26.52, 36.87, 43.99];
hit_rate_lrfu = [17.24, 21.23, 23.13, 31.56, 36.31];

figure;
hold on; grid on;

%Logarithmic scale
semilogx(capacities, hit_rate_opt,  '-k^', 'LineWidth', 2, 'MarkerSize', 8); 
semilogx(capacities, hit_rate_arc,  '-ro', 'LineWidth', 2, 'MarkerSize', 8); 
semilogx(capacities,hit_rate_2q,   '-cx', 'LineWidth', 2, 'MarkerSize', 8); 
semilogx(capacities, hit_rate_lirs, '-bs', 'LineWidth', 2, 'MarkerSize', 8); 
semilogx(capacities, hit_rate_lru,  '-md', 'LineWidth', 2, 'MarkerSize', 8); 
semilogx(capacities, hit_rate_lfuk, '-yv', 'LineWidth', 2, 'MarkerSize', 8); 
semilogx(capacities, hit_rate_lfu,  '-g*', 'LineWidth', 2, 'MarkerSize', 8); 
semilogx(capacities, hit_rate_lrfu, '-p',  'Color', [1 0.5 0], 'LineWidth', 2, 'MarkerSize', 8);
set(gca, 'XScale', 'log');
xticks(capacities);
xticklabels({'0.05%', '0.5%', '1%', '5%', '10%'});

xlabel('Cache Capacity (%)', 'FontSize', 12, 'FontWeight', 'bold');
ylabel('Hit Rate (%)', 'FontSize', 12, 'FontWeight', 'bold');
title('Hit Rate vs Cache Capacity (meta\_rprn)', 'FontSize', 14);


legend('OPT', 'ARC', '2Q', 'LIRS', 'LRU', 'LFU-k', 'LFU', 'LRFU', ...
    'Location', 'northwest', 'FontSize', 11);

hold off;