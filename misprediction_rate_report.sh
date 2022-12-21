#!/bin/bash

echoerr() { echo "$@" 1>&2; }

# Capture misprediction rate and report exit code of the command.
mis_rate() {
    local cmd=$@
    output="$($cmd)"
    if [ $? -ne 0 ]; then
        echoerr "Error running sim with $cmd"
        return 255
    fi

    # Extract misprediction rate from output. Looks for "misprediction rate: X%"
    misprediction_rate="$(echo "$output" | grep "misprediction rate" | cut -d ":" -f 2 | tr -d ' ' | tr -d '\t')"
    echo "$misprediction_rate"
}

TRACE_FILES=("./tests/gcc_trace.txt" "./tests/perl_trace.txt" "./tests/jpeg_trace.txt")
REPORTS_DIR="./reports"

mkdir -p "$REPORTS_DIR"

# Run n-bit smith predictor with different counter bit sizes.
for t in "${TRACE_FILES[@]}"; do
    tn="$(basename "$t")" || exit $?
    data="$(for c in 1 2 3 4 5 6; do
        mis_rate="$(mis_rate ./sim smith "$c" "$t")" || exit $?
        echo "$c,$mis_rate"
    done)"

    # IMPORTANT: bash here string are indented with tabs.
    read -r -d '' gnuplot <<-EOF
		\$data <<EOD
		$data
		EOD
		set term pngcairo;
		set datafile separator ',';
		set xlabel 'B (Counter Bits)';
		set ylabel 'Misprediction Percentage';
		set title 'Smith n-bit branch predictor for trace file $tn' noenhance;
		plot '\$data' with linespoints notitle';
	EOF
    echo "$gnuplot" | gnuplot -p > "$REPORTS_DIR/$tn,SMITH.png"
done

# Run bimodal predictor with different prediction table sizes.
for t in "${TRACE_FILES[@]}"; do
    tn="$(basename "$t")" || exit $?
    data="$(for m in 7 8 9 10 11 12; do
        mis_rate="$(mis_rate ./sim bimodal "$m" "$t")" || exit $?
        echo "$m,$mis_rate"
    done)"

    # IMPORTANT: bash here string are indented with tabs.
    read -r -d '' gnuplot <<-EOF
		\$data <<EOD
		$data
		EOD
		set term pngcairo;
		set datafile separator ',';
		set xlabel 'M (PC Bits)';
		set ylabel 'Misprediction Percentage';
		set title 'Bimodal branch predictor for trace file $tn' noenhance;
		plot '\$data' with linespoints notitle';
	EOF
    echo "$gnuplot" | gnuplot -p > "$REPORTS_DIR/$tn,BIMODAL.png"
done

# Run gshare predictor with different prediction table sizes and global branch
# history register size.
for t in "${TRACE_FILES[@]}"; do
    tn="$(basename "$t")" || exit $?

    data="$(for m in 7 8 9 10 11 12; do
        echo -n "$m,"
        for (( n=2; n<=m; n+=2 )); do
            mis_rate="$(mis_rate ./sim gshare "$m" "$n" "$t")" || exit $?
            if (( m-n >= 2)); then
                echo -n "$mis_rate,"
            else
                echo "$mis_rate"
            fi
        done
    done)"

    # IMPORTANT: bash here string are indented with tabs.
    read -r -d '' gnuplot <<-EOF
		\$data <<EOD
		$data
		EOD
		set term pngcairo;
		set datafile separator ',';
		set xlabel 'M (PC Bits)';
		set ylabel 'Misprediction Percentage';
		set title 'Gshare branch predictor for trace file $tn' noenhance;
		plot '\$data' using 1:2 with linespoints title 'N=2 (global branch history register bits)' \
            , '' using 1:3 with linespoints title 'N=4' \
			, '' using 1:4 with linespoints title 'N=6' \
			, '' using 1:5 with linespoints title 'N=8' \
			, '' using 1:6 with linespoints title 'N=10' \
			, '' using 1:7 with linespoints title 'N=12';
	EOF
    echo "$gnuplot" | gnuplot -p > "$REPORTS_DIR/$tn,GSHARE.png"
done
