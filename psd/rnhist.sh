./rp_read -n 5000 -i 5000
echo 'mv hist_sum.csv hist_sum_5k_05k.csv'
mv hist_sum.csv hist_sum_5k_05k.csv
mv sums.csv sum_5k_05k.csv

./rp_read -n 5000 -i 10000
echo 'mv hist_sum.csv hist_sum_5k_10k.csv'
mv hist_sum.csv hist_sum_5k_10k.csv
mv sums.csv sum_5k_10k.csv

./rp_read -n 5000 -i 15000
echo 'mv hist_sum.csv hist_sum_5k_15k.csv'
mv hist_sum.csv hist_sum_5k_15k.csv
mv sums.csv sum_5k_15k.csv

./rp_read -n 5000 -i 20000
echo 'mv hist_sum.csv hist_sum_5k_20k.csv'
mv hist_sum.csv hist_sum_5k_20k.csv
mv sums.csv sum_5k_20k.csv

./rp_read -n 10000 -i 5000
echo 'mv hist_sum.csv hist_sum_10k_05k.csv'
mv hist_sum.csv hist_sum_10k_05k.csv
mv sums.csv sum_5k_05k.csv

./rp_read -n 10000 -i 10000
echo 'mv hist_sum.csv hist_sum_10k_10k.csv'
mv hist_sum.csv hist_sum_10k_10k.csv
mv sums.csv sum_5k_10k.csv

./rp_read -n 10000 -i 15000
echo 'mv hist_sum.csv hist_sum_10k_15k.csv'
mv hist_sum.csv hist_sum_10k_15k.csv
mv sums.csv sum_5k_15k.csv

./rp_read -n 10000 -i 20000
echo 'mv hist_sum.csv hist_sum_10k_20k.csv'
mv hist_sum.csv hist_sum_10k_20k.csv
mv sums.csv sum_5k_20k.csv
