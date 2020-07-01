for cpu in $(seq 1 79);
do
	{ sudo cpufreq-set -g ondemand -c $cpu; }
done;

echo "CPU Free..."