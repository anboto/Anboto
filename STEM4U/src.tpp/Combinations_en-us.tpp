topic "Combinations";
[H6;0 $$1,0#05600065144404261032431302351956:begin]
[i448;a25;kKO9;2 $$2,0#37138531426314131252341829483370:codeitem]
[l288;2 $$3,0#27521748481378242620020725143825:desc]
[0 $$4,0#96390100711032703541132217272105:end]
[ $$0,0#00000000000000000000000000000000:Default]
[{_} 
[ {{10000@3 [s0;%% [*@7;4 Combinations]]}}&]
[s1; &]
[s0; &]
[s0;%% [2 Permutation and combinations functions.]&]
[s0;2%% &]
[s0;%% [2 These follow a for`_each style: The algorithm calls a user 
supplied function object (functor) for each combination/permutation 
in the sequence: f(begin, end). That functor can maintain state. 
The sequence need not be sorted, nor even contain unique objects. 
The algorithms do not consider the value of the sequence elements 
at all. That is, the element type need not support LessThanComparable 
nor EqualityComparable. Furthermore the algorithms follow three 
additional rules:]&]
[s0;2%% &]
[s0;i150;O0;%% [2 On normal (non`-exceptional) completion, the sequence 
is always left in the original order.]&]
[s0;i150;O0;%% [2 The functor is always called with (first, mid). This 
enables the functor to also access the elements not in the sequence 
if it is aware of the sequence: (mid, last). This can come in 
handy when dealing with nested combination/permutation problems 
where for each permutation you also need to compute combinations 
and/or permutations on those elements not selected.]&]
[s0;i150;O0;%% [2 The functor should return true or false: true if the 
functor wishes to break out of the for`_each`_ loop, and otherwise 
false.]&]
[s0;2%% &]
[s0;%% [2 Each of the functions return the number of combinations or 
zero in case of wrong input or overflow.]&]
[s0;2%% &]
[s0;2%% &]
[s4;%% &]
[s1; &]
[s2;:Upp`:`:ForEachPermutation`(T`&`,int`,Functor`): [@(0.0.255) template]_<[@(0.0.255) c
lass]_[*@4 T], [@(0.0.255) class]_[*@4 Functor]>_[_^Upp`:`:uint64^ uint64]_[* ForEachPermut
ation]([*@4 T]_`&[*@3 data], [@(0.0.255) int]_[*@3 mid], [*@4 Functor]_[*@3 f])&]
[s3; Repeatedly permutes the range `[[*@3 data], [*@3 data.end]()) such 
that the range `[[*@3 data], [*@3 mid]) represents each permutation 
of the values in `[[*@3 data], [*@3 data.end]()) taken distance ([*@3 data], 
[*@3 mid]) at a time. &]
[s3; For each permutation calls [*@3 f]([*@3 data], [*@3 mid]). On each 
call, the range `[[*@3 mid], [*@3 data.end]()) holds the values not 
in the current permutation. If [*@3 f] returns true then returns 
immediately without permuting the sequence any further. Otherwise, 
after the last call to [*@3 f], and prior to returning, the range 
`[[*@3 data], [*@3 data.end]()) is restored to its original order. 
`[Note: If [*@3 f] always returns false it is called [* CountEachPermutation]([*@3 data],
 [*@3 mid]) times`]&]
[s3; [@N Returns: ][*@3 f][@N .]&]
[s4;%% &]
[s1; &]
[s2;:Upp`:`:ForEachPermutation`(T`&`,Functor`): [@(0.0.255) template]_<[@(0.0.255) class]_
[*@4 T], [@(0.0.255) class]_[*@4 Functor]>_[_^Upp`:`:uint64^ uint64]_[* ForEachPermutation](
[*@4 T]_`&[*@3 data], [*@4 Functor]_[*@3 f])&]
[s3; Repeatedly permutes the range `[[*@3 data], [*@3 data.end]()).&]
[s3; For each permutation calls [*@3 f]([*@3 data], [*@3 data.end]()).&]
[s3; If [*@3 f] returns true then returns immediately without permuting 
the sequence any further. Otherwise, after the last call to [*@3 f], 
and prior to returning, the range `[[*@3 data], [*@3 data.end]()) 
is restored to its original order. `[Note: If [*@3 f] always returns 
false it is called [* CountEachPermutation]([*@3 data]) times`]&]
[s3; [@N Returns: ][*@3 f][@N .]&]
[s4;%% &]
[s1; &]
[s2;:Upp`:`:ForEachReversiblePermutation`(T`&`,int`,Functor`): [@(0.0.255) template]_<[@(0.0.255) c
lass]_[*@4 T], [@(0.0.255) class]_[*@4 Functor]>_[_^Upp`:`:uint64^ uint64]_[* ForEachRevers
iblePermutation]([*@4 T]_`&[*@3 data], [@(0.0.255) int]_[*@3 mid], [*@4 Functor]_[*@3 f])&]
[s3; Repeatedly permutes the range `[[*@3 data], [*@3 data.end]()) such 
that the range `[[*@3 data], [*@3 mid]) represents each permutation 
of the values in `[[*@3 data], [*@3 data.end]()) taken distance ([*@3 data], 
[*@3 mid]) at a time, [_ except that ][*_@3 f][_  is never called with 
the reverse of a permutation which has been previously called.] 
&]
[s3; For each permutation calls [*@3 f]([*@3 data], [*@3 mid]). On each 
call, the range `[[*@3 mid], [*@3 data.end]()) holds the values not 
in the current permutation. If [*@3 f] returns true then returns 
immediately without permuting the sequence any further. Otherwise, 
after the last call to [*@3 f], and prior to returning, the range 
`[[*@3 data], [*@3 data.end]()) is restored to its original order. 
`[Note: If [*@3 f] always returns false it is called [* CountEachReversiblePermutation
]([*@3 data], [*@3 mid]) times`]&]
[s3; [@N Returns: ][*@3 f][@N .]&]
[s4;%% &]
[s1; &]
[s2;:Upp`:`:ForEachReversiblePermutation`(T`&`,Functor`): [@(0.0.255) template]_<[@(0.0.255) c
lass]_[*@4 T], [@(0.0.255) class]_[*@4 Functor]>_[_^Upp`:`:uint64^ uint64]_[* ForEachRevers
iblePermutation]([*@4 T]_`&[*@3 data], [*@4 Functor]_[*@3 f])&]
[s3; Repeatedly permutes the range `[[*@3 data], [*@3 data.end]()), [_ except 
that ][*_@3 f][_  is never called with the reverse of a permutation 
which has been previously called.] &]
[s3; For each permutation calls [*@3 f]([*@3 data], [*@3 data.end]()).&]
[s3; If [*@3 f] returns true then returns immediately without permuting 
the sequence any further. Otherwise, after the last call to [*@3 f], 
and prior to returning, the range `[[*@3 data], [*@3 data.end]()) 
is restored to its original order. `[Note: If [*@3 f] always returns 
false it is called [* CountEachReversiblePermutation]([*@3 data]) 
times`]&]
[s3; [@N Returns: ][*@3 f][@N .]&]
[s4;%% &]
[s1; &]
[s2;:Upp`:`:ForEachCircularPermutation`(T`&`,int`,Functor`): [@(0.0.255) template]_<[@(0.0.255) c
lass]_[*@4 T], [@(0.0.255) class]_[*@4 Functor]>_[_^Upp`:`:uint64^ uint64]_[* ForEachCircul
arPermutation]([*@4 T]_`&[*@3 data], [@(0.0.255) int]_[*@3 mid], [*@4 Functor]_[*@3 f])&]
[s3; Repeatedly permutes the range `[[*@3 data], [*@3 data.end]()) such 
that the range `[[*@3 data], [*@3 mid]) represents each permutation 
of the values in `[[*@3 data], [*@3 data.end]()) taken distance ([*@3 data], 
[*@3 mid]) at a time, [_ except that ][*_@3 f][_  is never called with 
a circular permutation which has been previously called. ]&]
[s3; For each permutation calls [*@3 f]([*@3 data], [*@3 mid]). On each 
call, the range `[[*@3 mid], [*@3 data.end]()) holds the values not 
in the current permutation. If [*@3 f] returns true then returns 
immediately without permuting the sequence any further. Otherwise, 
after the last call to [*@3 f], and prior to returning, the range 
`[[*@3 data], [*@3 data.end]()) is restored to its original order. 
`[Note: If [*@3 f] always returns false it is called [* CountEachCircularPermutation](
[*@3 data], [*@3 mid]) times`]&]
[s3; [@N Returns: ][*@3 f][@N .]&]
[s4;%% &]
[s1; &]
[s2;:Upp`:`:ForEachCircularPermutation`(T`&`,Functor`): [@(0.0.255) template]_<[@(0.0.255) c
lass]_[*@4 T], [@(0.0.255) class]_[*@4 Functor]>_[_^Upp`:`:uint64^ uint64]_[* ForEachCircul
arPermutation]([*@4 T]_`&[*@3 data], [*@4 Functor]_[*@3 f])&]
[s3; Repeatedly permutes the range `[[*@3 data], [*@3 data.end]()), [_ except 
that ][*_@3 f][_  is never called with a circular permutation which 
has been previously called.] &]
[s3; For each permutation calls [*@3 f]([*@3 data], [*@3 data.end]()).&]
[s3; If [*@3 f] returns true then returns immediately without permuting 
the sequence any further. Otherwise, after the last call to [*@3 f], 
and prior to returning, the range `[[*@3 data], [*@3 data.end]()) 
is restored to its original order. `[Note: If [*@3 f] always returns 
false it is called [* CountEachCircularPermutation]([*@3 data]) times`]&]
[s3; [@N Returns: ][*@3 f][@N .]&]
[s4;%% &]
[s1; &]
[s2;:Upp`:`:ForEachReversibleCircularPermutation`(T`&`,int`,Functor`): [@(0.0.255) temp
late]_<[@(0.0.255) class]_[*@4 T], [@(0.0.255) class]_[*@4 Functor]>_[_^Upp`:`:uint64^ uint
64]_[* ForEachReversibleCircularPermutation]([*@4 T]_`&[*@3 data], 
[@(0.0.255) int]_[*@3 mid], [*@4 Functor]_[*@3 f])&]
[s3; Repeatedly permutes the range `[[*@3 data], [*@3 data.end]()) such 
that the range `[[*@3 data], [*@3 mid]) represents each permutation 
of the values in `[[*@3 data], [*@3 data.end]()) taken distance ([*@3 data], 
[*@3 mid]) at a time, [_ except that ][*_@3 f][_  is never called with 
a circular permutation which has been previously called, or the 
reverse of that permutation. ]&]
[s3; For each permutation calls [*@3 f]([*@3 data], [*@3 mid]). On each 
call, the range `[[*@3 mid], [*@3 data.end]()) holds the values not 
in the current permutation. If [*@3 f] returns true then returns 
immediately without permuting the sequence any further. Otherwise, 
after the last call to [*@3 f], and prior to returning, the range 
`[[*@3 data], [*@3 data.end]()) is restored to its original order. 
`[Note: If [*@3 f] always returns false it is called [* CountEachCircularPermutation](
[*@3 data], [*@3 mid]) times`]&]
[s3; [@N Returns: ][*@3 f][@N .]&]
[s4;%% &]
[s1; &]
[s2;:Upp`:`:ForEachReversibleCircularPermutation`(T`&`,Functor`): [@(0.0.255) template]_
<[@(0.0.255) class]_[*@4 T], [@(0.0.255) class]_[*@4 Functor]>_[_^Upp`:`:uint64^ uint64]_[* F
orEachReversibleCircularPermutation]([*@4 T]_`&[*@3 data], [*@4 Functor]_[*@3 f])&]
[s3; Repeatedly permutes the range `[[*@3 data], [*@3 data.end]()), [_ except 
that ][*_@3 f][_  is never called with a circular permutation which 
has been previously called, or the reverse of that permutation. 
]&]
[s3; For each permutation calls [*@3 f]([*@3 data], [*@3 data.end]()).&]
[s3; If [*@3 f] returns true then returns immediately without permuting 
the sequence any further. Otherwise, after the last call to [*@3 f], 
and prior to returning, the range `[[*@3 data], [*@3 data.end]()) 
is restored to its original order. `[Note: If [*@3 f] always returns 
false it is called [* CountEachReversibleCircularPermutation]([*@3 data]) 
times`]&]
[s3; [@N Returns: ][*@3 f][@N .]&]
[s4;%% &]
[s1; &]
[s2;:Upp`:`:ForEachCombination`(T`&`,int`,Functor`): [@(0.0.255) template]_<[@(0.0.255) c
lass]_[*@4 T], [@(0.0.255) class]_[*@4 Functor]>_[_^Upp`:`:uint64^ uint64]_[* ForEachCombin
ation]([*@4 T]_`&[*@3 data], [@(0.0.255) int]_[*@3 mid], [*@4 Functor]_[*@3 f])&]
[s3; Repeatedly permutes the range `[[*@3 data], [*@3 data.end]()) such 
that the range `[[*@3 data], [*@3 mid]) represents each combination 
of the values in `[[*@3 data], [*@3 data.end]()) taken distance ([*@3 data], 
[*@3 mid]) at a time.&]
[s3; For each permutation calls [*@3 f]([*@3 data], [*@3 mid]). On each 
call, the range `[[*@3 mid], [*@3 data.end]()) holds the values not 
in the current permutation. If [*@3 f] returns true then returns 
immediately without permuting the sequence any further. Otherwise, 
after the last call to [*@3 f], and prior to returning, the range 
`[[*@3 data], [*@3 data.end]()) is restored to its original order. 
`[Note: If [*@3 f] always returns false it is called [* CountEachCombination]([*@3 data],
 [*@3 mid]) times`]&]
[s3; [@N Returns: ][*@3 f][@N .]&]
[s4;%% &]
[s1; &]
[s2;:Upp`:`:ForEachCombination`(T`&`,Functor`): [@(0.0.255) template]_<[@(0.0.255) class]_
[*@4 T], [@(0.0.255) class]_[*@4 Functor]>_[_^Upp`:`:uint64^ uint64]_[* ForEachCombination](
[*@4 T]_`&[*@3 data], [*@4 Functor]_[*@3 f])&]
[s3; Repeatedly combines the range `[[*@3 data], [*@3 data.end]()).&]
[s3; For each permutation calls [*@3 f]([*@3 data], [*@3 data.end]()).&]
[s3; If [*@3 f] returns true then returns immediately without permuting 
the sequence any further. Otherwise, after the last call to [*@3 f], 
and prior to returning, the range `[[*@3 data], [*@3 data.end]()) 
is restored to its original order. `[Note: If [*@3 f] always returns 
false it is called [* CountEachReversibleCircularPermutation]([*@3 data]) 
times`]&]
[s3; [@N Returns: ][*@3 f][@N .]&]
[s4;%% &]
[s1; &]
[s2;:Upp`:`:CountEachPermutation`(const T`&`,int`): [@(0.0.255) template]_<[@(0.0.255) cl
ass]_[*@4 T]>_[_^Upp`:`:uint64^ uint64]_[* CountEachPermutation]([@(0.0.255) const]_[*@4 T]_
`&[*@3 data], [@(0.0.255) int]_[*@3 mid])&]
[s3;%% Returns [%-* CountEachPermutation][%- (std`::distance(][%-*@3 data][%- , 
][%-*@3 mid][%- ), std`::distance(][%-*@3 mid][%- , ][%-*@3 data.end][%- ()))] 
.&]
[s4;%% &]
[s1; &]
[s2;:Upp`:`:CountEachPermutation`(const T`&`): [@(0.0.255) template]_<[@(0.0.255) class]_
[*@4 T]>_uint64_[* CountEachPermutation]([@(0.0.255) const]_[*@4 T]_`&[*@3 data])&]
[s3;%% Returns [%-* CountEachPermutation][%- (std`::distance(][%-*@3 data][%- , 
][%-*@3 data.end][%- ()), std`::distance(][%-*@3 data][%- , ][%-*@3 data.end][%- ()))] 
&]
[s4;%% &]
[s1; &]
[s2;:Upp`:`:CountEachReversiblePermutation`(const T`&`,int`): [@(0.0.255) template]_<[@(0.0.255) c
lass]_[*@4 T]>_[_^Upp`:`:uint64^ uint64]_[* CountEachReversiblePermutation]([@(0.0.255) c
onst]_[*@4 T]_`&[*@3 data], [@(0.0.255) int]_[*@3 mid])&]
[s3;%% Returns [%-* CountEachReversiblePermutation][%- (std`::distance(][%-*@3 data][%- , 
][%-*@3 mid][%- ), std`::distance(][%-*@3 mid][%- , ][%-*@3 data.end][%- ()))] 
.&]
[s4;%% &]
[s1; &]
[s2;:Upp`:`:CountEachReversiblePermutation`(const T`&`): [@(0.0.255) template]_<[@(0.0.255) c
lass]_[*@4 T]>_uint64_[* CountEachReversiblePermutation]([@(0.0.255) const]_[*@4 T]_`&[*@3 d
ata])&]
[s3;%% Returns [%-* CountEachReversiblePermutation][%- (std`::distance(][%-*@3 data][%- , 
][%-*@3 data.end][%- ()), std`::distance(][%-*@3 data][%- , ][%-*@3 data.end][%- ()))] 
&]
[s4;%% &]
[s1; &]
[s2;:Upp`:`:CountEachCircularPermutation`(const T`&`,int`): [@(0.0.255) template]_<[@(0.0.255) c
lass]_[*@4 T]>_[_^Upp`:`:uint64^ uint64]_[* CountEachCircularPermutation]([@(0.0.255) con
st]_[*@4 T]_`&[*@3 data], [@(0.0.255) int]_[*@3 mid])&]
[s3;%% Returns [%-* CountEachCircularPermutation][%- (std`::distance(][%-*@3 data][%- , 
][%-*@3 mid][%- ), std`::distance(][%-*@3 mid][%- , ][%-*@3 data.end][%- ()))] 
.&]
[s4;%% &]
[s1; &]
[s2;:Upp`:`:CountEachCircularPermutation`(const T`&`): [@(0.0.255) template]_<[@(0.0.255) c
lass]_[*@4 T]>_uint64_[* CountEachCircularPermutation]([@(0.0.255) const]_[*@4 T]_`&[*@3 da
ta])&]
[s3;%% Returns [%-* CountEachCircularPermutation][%- (std`::distance(][%-*@3 data][%- , 
][%-*@3 data.end][%- ()), std`::distance(][%-*@3 data][%- , ][%-*@3 data.end][%- ()))] 
&]
[s4;%% &]
[s1; &]
[s2;:Upp`:`:CountEachReversibleCircularPermutation`(const T`&`,int`): [@(0.0.255) templ
ate]_<[@(0.0.255) class]_[*@4 T]>_[_^Upp`:`:uint64^ uint64]_[* CountEachReversibleCircula
rPermutation]([@(0.0.255) const]_[*@4 T]_`&[*@3 data], [@(0.0.255) int]_[*@3 mid])&]
[s3;%% Returns [%-* CountEachReversibleCircularPermutation][%- (std`::distance(][%-*@3 data][%- ,
 ][%-*@3 mid][%- ), std`::distance(][%-*@3 mid][%- , ][%-*@3 data.end][%- ()))] 
.&]
[s4;%% &]
[s1; &]
[s2;:Upp`:`:CountEachReversibleCircularPermutation`(const T`&`): [@(0.0.255) template]_
<[@(0.0.255) class]_[*@4 T]>_uint64_[* CountEachReversibleCircularPermutation]([@(0.0.255) c
onst]_[*@4 T]_`&[*@3 data])&]
[s3;%% Returns [%-* CountEachReversibleCircularPermutation][%- (std`::distance(][%-*@3 data][%- ,
 ][%-*@3 data.end][%- ()), std`::distance(][%-*@3 data][%- , ][%-*@3 data.end][%- ()))] 
&]
[s4;%% &]
[s1; &]
[s2;:Upp`:`:CountEachCombinationPermutation`(const T`&`,int`): [@(0.0.255) template]_<[@(0.0.255) c
lass]_[*@4 T]>_[_^Upp`:`:uint64^ uint64]_[* CountEachCombinationPermutation]([@(0.0.255) c
onst]_[*@4 T]_`&[*@3 data], [@(0.0.255) int]_[*@3 mid])&]
[s3;%% Returns [%-* CountEachCombinationPermutation][%- (std`::distance(][%-*@3 data][%- , 
][%-*@3 mid][%- ), std`::distance(][%-*@3 mid][%- , ][%-*@3 data.end][%- ()))] 
.&]
[s4;%% &]
[s1; &]
[s2;:Upp`:`:CountEachCombinationPermutation`(const T`&`): [@(0.0.255) template]_<[@(0.0.255) c
lass]_[*@4 T]>_uint64_[* CountEachCombinationPermutation]([@(0.0.255) const]_[*@4 T]_`&[*@3 d
ata])&]
[s3;%% Returns [%-* CountEachCombinationPermutation][%- (std`::distance(][%-*@3 data][%- , 
][%-*@3 data.end][%- ()), std`::distance(][%-*@3 data][%- , ][%-*@3 data.end][%- ()))] 
&]
[s4;%% &]
[s1; &]
[s2;:Upp`:`:CountEachPermutation`(UInt`,UInt`): [@(0.0.255) template]_<[@(0.0.255) class]_
[*@4 UInt]>_[*@4 UInt]_[* CountEachPermutation]([*@4 UInt]_[*@3 d1], [*@4 UInt]_[*@3 d2])&]
[s3;%% Returns ([%-*@3 d1] `+ [%-*@3 d2]!)/[%-*@3 d2]!.&]
[s3;%% If the computed value is not representable in the type [%-*@4 UInt], 
returns 0.&]
[s3;%% If [%-*@3 d1] < [%-*@4 UInt](0) or [%-*@3 d2 ]< [%-*@4 UInt](0), returns 
0. &]
[s4;%% &]
[s1; &]
[s2;:Upp`:`:CountEachReversiblePermutation`(UInt`,UInt`): [@(0.0.255) template]_<[@(0.0.255) c
lass]_[*@4 UInt]>_[*@4 UInt]_[* CountEachReversiblePermutation]([*@4 UInt]_[*@3 d1], 
[*@4 UInt]_[*@3 d2])&]
[s3; [@N Returns: If ][*@3 d1][%%  ][@N <`= 1 returns (][*@3 d1][%%  ][@N `+ ][*@3 d2][@N )!/][*@3 d2][@N !.
 Else returns (][*@3 d1][%%  ][@N `+ ][*@3 d2][@N )!/(2`*][*@3 d2][@N !).]&]
[s3;%% If the computed value is not representable in the type [%-*@4 UInt], 
returns 0.&]
[s3;%% If [%-*@3 d1] < [%-*@4 UInt](0) or [%-*@3 d2 ]< [%-*@4 UInt](0), returns 
0. &]
[s4;%% &]
[s1; &]
[s2;:Upp`:`:CountEachCircularPermutation`(UInt`,UInt`): [@(0.0.255) template]_<[@(0.0.255) c
lass]_[*@4 UInt]>_[*@4 UInt]_[* CountEachCircularPermutation]([*@4 UInt]_[*@3 d1], 
[*@4 UInt]_[*@3 d2])&]
[s3;%% [%-@N If ][%-*@3 d1] [%-@N `=`= 0 returns 1. Else returns (][%-*@3 d1] 
[%-@N `+ ][%-*@3 d2][%-@N )!/(][%-*@3 d1][%-@N `*][%-*@3 d2][%-@N !)].&]
[s3;%% If the computed value is not representable in the type [%-*@4 UInt], 
returns 0.&]
[s3;%% If [%-*@3 d1] < [%-*@4 UInt](0) or [%-*@3 d2 ]< [%-*@4 UInt](0), returns 
0. &]
[s4;%% &]
[s1; &]
[s2;:Upp`:`:CountEachReversibleCircularPermutation`(UInt`,UInt`): [@(0.0.255) template]_
<[@(0.0.255) class]_[*@4 UInt]>_[*@4 UInt]_[* CountEachReversibleCircularPermutation]([*@4 U
Int]_[*@3 d1], [*@4 UInt]_[*@3 d2])&]
[s3; [@N If ][*@3 d1 ][@N `=`= 0 returns 1. Else if ][*@3 d1][@N <`= 2 returns 
(][*@3 d1][%%  ][@N `+ ][*@3 d2][@N )!/(][*@3 d1][@N `*][*@3 d2][@N !). Else returns 
(][*@3 d1][@N `+ ][*@3 d2][@N )!/(2`*][*@3 d1][@N `*][*@3 d2][@N !).]&]
[s3;%% If the computed value is not representable in the type [%-*@4 UInt], 
returns 0.&]
[s3;%% If [%-*@3 d1] < [%-*@4 UInt](0) or [%-*@3 d2 ]< [%-*@4 UInt](0), returns 
0. &]
[s4;%% &]
[s1; &]
[s2;:Upp`:`:CountEachCombination`(UInt`,UInt`): [@(0.0.255) template]_<[@(0.0.255) class]_
[*@4 UInt]>_[*@4 UInt]_[* CountEachCombination]([*@4 UInt]_[*@3 d1], [*@4 UInt]_[*@3 d2])&]
[s3; [@N Returns: (][*@3 d1 ][@N `+ ][*@3 d2][@N )!/(][*@3 d1][@N !`*][*@3 d2][@N !). 
]&]
[s3;%% If the computed value is not representable in the type [%-*@4 UInt], 
returns 0.&]
[s3;%% If [%-*@3 d1] < [%-*@4 UInt](0) or [%-*@3 d2 ]< [%-*@4 UInt](0), returns 
0. &]
[s4;%% ]]