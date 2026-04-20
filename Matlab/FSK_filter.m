classdef FSK_filter

    properties
        BUF_SZ      {mustBeNumeric}
        adc_buf     {mustBeNumeric}
        adc_ptr     {mustBeNumeric}
        calc_buf    {mustBeNumeric}
        calc_ptr    {mustBeNumeric}
        
        % Variable to choose if convolution or
        % Dot-Product demodulation is used
        demod_proc

        % Filter Functions
        s0_sin     {mustBeNumeric}
        s0_cos     {mustBeNumeric}
        s1_sin     {mustBeNumeric}
        s1_cos     {mustBeNumeric}
        I_0        {mustBeNumeric}
        Q_0        {mustBeNumeric}
        I_1        {mustBeNumeric}
        Q_1        {mustBeNumeric}

        % Output Values
        y0     {mustBeNumeric}
        y1     {mustBeNumeric}

        % Bit Detection
        skip_Ts_idle     {mustBeNumeric}
        skip_Ts_idle_CNT {mustBeNumeric}

        threshold_high   {mustBeNumeric}
        threshold_low    {mustBeNumeric}

        T_bit_Counter    {mustBeNumeric}

        signal_detected  {mustBeNumeric}
                
        bit_cnt          {mustBeInteger}
        byte             {mustBeInteger}

    end

    methods
        function obj = FSK_filter(T_bit, Fs, f0, f1, A, thresh_h, thresh_l, skip_Ts_idle, demod_proc)
            % calculate filter function
            k = (T_bit*Fs - 1):-1:0;
            t = k/Fs;

            obj.s0_sin = A*sin(2*pi*f0*t);
            obj.s0_cos = A*cos(2*pi*f0*t);
            obj.s1_sin = A*sin(2*pi*f1*t);
            obj.s1_cos = A*cos(2*pi*f1*t);
            
            obj.demod_proc = demod_proc;
            
            obj.I_0 = 0;
            obj.Q_0 = 0;
            obj.I_1 = 0;
            obj.Q_1 = 0;

            % init buffers
            obj.BUF_SZ = T_bit*Fs;
            obj.adc_buf = zeros(1, obj.BUF_SZ);
            obj.adc_ptr = 1;
            obj.calc_buf = zeros(1, obj.BUF_SZ);
            obj.calc_ptr = 1;
            
            % init Outputs
            obj.y0 = 0;
            obj.y1 = 0;
            
            % init Bit detection
            obj.skip_Ts_idle = skip_Ts_idle;
            obj.skip_Ts_idle_CNT = 0;

            obj.threshold_high = thresh_h;
            obj.threshold_low  = thresh_l;

            obj.T_bit_Counter = 0;
            
            obj.signal_detected = 0;

            obj.bit_cnt = 0;
            obj.byte = 0;
        end

        function obj = addVal(obj, adc_val)
            % Add new value to adc buffer
            obj.adc_buf(obj.adc_ptr) = adc_val;
            
            % if buffer is full, reset pointer/index (circular)
            obj.adc_ptr = obj.adc_ptr + 1;
            if obj.adc_ptr > obj.BUF_SZ
                obj.adc_ptr = 1;
            end
            
            % I have no clue why thats here, it seems unnecessary
            if obj.signal_detected == 1
                obj.T_bit_Counter = obj.T_bit_Counter;
            end
        end

        function obj = conv(obj)

            % Circular ADC-Buffer has elements not in order in respect to time
            % f.e: [5 6 7 8 9 1 2 3 4]
            %                 ^
            %                 | adc_pointer
            % correct data in respect to time would be [1 2 3 ... 9]
            % with indices: [(adc_pointer) ... (BUF_SZ) (0) ... (BUF_SZ - 1)]
            %
            % To calculate convolution with matlabs conv() or filter(), we're copying adc data
            % to a dedicated calculation array where the data is in the right order
            obj.calc_ptr = obj.adc_ptr;
            for j=1:1:obj.BUF_SZ
                obj.calc_buf(j) = obj.adc_buf(obj.calc_ptr);
                obj.calc_ptr = obj.calc_ptr + 1;
                if obj.calc_ptr > obj.BUF_SZ
                    obj.calc_ptr = 1;
                end
            end


            % For both filters, if signal has not been detected then the
            % filter output needs to reach threshold_high, afterwards
            % (signal detected) it only needs to be higher then threshold
            % low (should be a bit smaller than high), because the at the
            % next T_bit conv() it could be a bit smaller than
            % threshold_high
                
            if strcmp(obj.demod_proc, 'conv')
                % filter() and conv() interchangably
                % obj.y0 = rms(filter(obj.s0_sin, 1, obj.calc_buf));
                obj.y0 = rms(conv(obj.calc_buf, obj.s0_sin));
            else
                obj.I_0 = dot(obj.calc_buf, obj.s0_sin);
                obj.Q_0 = dot(obj.calc_buf, obj.s0_cos);
                obj.y0 = obj.I_0^2 + obj.Q_0^2;
            end
            if (obj.signal_detected == 0 && obj.y0 >= obj.threshold_high) || ...
               (obj.signal_detected == 1 && obj.y0 >= obj.threshold_low)
                if obj.signal_detected == 0
                    obj.signal_detected = 1;
                end
                
                % Bitshift a 0 into the current bit position (kinda
                % unnecessary), and or it together with the complete byte
                obj.byte = bitor(obj.byte, bitshift(0b0, 7-obj.bit_cnt));
                obj.bit_cnt = obj.bit_cnt + 1;
            end
            

            if strcmp(obj.demod_proc, 'conv')
                % filter() and conv() interchangably
                % obj.y1 = rms(filter(obj.s1_sin, 1, obj.calc_buf));
                obj.y1 = rms(conv(obj.calc_buf, obj.s1_sin));
            else
                obj.I_1 = dot(obj.calc_buf, obj.s1_sin);
                obj.Q_1 = dot(obj.calc_buf, obj.s1_cos);
                obj.y1 = obj.I_1^2 + obj.Q_1^2;
            end
            if (obj.signal_detected == 0 && obj.y1 >= obj.threshold_high) || ...
               (obj.signal_detected == 1 && obj.y1 >= obj.threshold_low)
                if obj.signal_detected == 0
                    obj.signal_detected = 1;
                end
                
                % Bitshift a 1 into the current bit position, 
                % and or it together with the complete byte
                obj.byte = bitor(obj.byte, bitshift(0b1, 7-obj.bit_cnt));
                obj.bit_cnt = obj.bit_cnt + 1;
            end
        
            % not 1 or 0 detected therefor transmission complete!
            if obj.y0 <= obj.threshold_low && obj.y1 <= obj.threshold_low
                obj.signal_detected = 0;
            end
        end

        function obj = update(obj, adc_val)
            % Add ADC Value into buffer
            obj = obj.addVal(adc_val);
            
            % Increment counter for skipping convolutions
            obj.skip_Ts_idle_CNT = obj.skip_Ts_idle_CNT + 1;
            obj.T_bit_Counter = obj.T_bit_Counter + 1;

            % If no signal has been detected wait for the skip_Ts_idle
            % Counter to reach the threshold, if it has been detected wait
            % for a whole T_bit to be reached
            % It just counts how often the update() method has been called
            % Not the best method for the microcontroller tbh, cause its
            % not given that the update call is 100% at every Ts (f.e.: ISR
            % takes longer than Ts)
            if (obj.signal_detected == 0 && obj.skip_Ts_idle_CNT >= obj.skip_Ts_idle) || ...
               (obj.signal_detected == 1 && obj.T_bit_Counter >= obj.BUF_SZ)
                obj.skip_Ts_idle_CNT = 0;
                obj.T_bit_Counter    = 0;
                obj = obj.conv();
            end
        end

        function x = isByteFinished(obj)
            % if the bit counter has reached 7 (all 8 bits collected)
            % return 1 (true) else its 0
            % sadly its not possible to reset bit_cnt if end is reached,
            % because if the instance of an object is changed it needs to
            % returned, and it cant return anything else (There would be
            % sth called "handler", which is sth like a c-pointer, but not
            % looked into it much)
            % So reseting bit_cnt to 0 is done after its called and did
            % return 1
            
            if obj.bit_cnt > 7
                x = 1;
                return
            end
            x = 0;
        end
    end
end