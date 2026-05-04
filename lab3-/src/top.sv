module top (
    input wire clk,

    input wire spi_sck,
    input wire spi_mosi,
    input wire spi_cs,

    output wire led0,
    output wire led1,
    output wire led2,
    output wire led3
);

reg spi_sck_prev;
reg [7:0] shift_reg;
reg [2:0] bit_count;
reg [3:0] leds;

always @(posedge clk) begin
    spi_sck_prev <= spi_sck;

    if (spi_cs == 1) begin
        bit_count <= 0;
    end
    else begin
        if (spi_sck_prev == 0 && spi_sck == 1) begin
            shift_reg <= {shift_reg[6:0], spi_mosi};

            if (bit_count == 7) begin
                leds <= {shift_reg[2:0], spi_mosi};
                bit_count <= 0;
            end
            else begin
                bit_count <= bit_count + 1;
            end
        end
    end
end

assign led0 = leds[0];
assign led1 = leds[1];
assign led2 = leds[2];
assign led3 = leds[3];

endmodule