for cpu in $(seq 1 79);
do
	{ sudo cpufreq-set -g userspace -c $cpu; }
done;

{ sudo cpupower frequency-set -f 3690000; }

echo "CPU Ready..."