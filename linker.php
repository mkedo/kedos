<?php


/*
 .text
 .data
 .bss
  
 В первом проходе собираем секции .text и .data 
 и проставляем им адреса относительно базового
 
 Во втором находим адреса для relocation
 */


$base_addr = 0x8200;
//xxx: align base address?


$coffs = array();

$coffs []= new Coff(__DIR__.'/build/entry.obj'); // must be the first element
foreach (glob(__DIR__."/build/obj/*.obj") as $file){
	$coffs []= new Coff($file);
}


$kernel_bin_file = __DIR__.'/build/kernel.bin';
$loader_file = __DIR__.'/build/obj/loader.bin';

// the final image
$kedos_file = __DIR__.'/build/kedos.img';




$section_align = 4;

$text_section_base = $base_addr;
$text_section_end = $text_section_base;
/**
 * array(
 * 	'addr' =>
 * 	'section' =>
 *  'pad_size' => байт для выравниваия к концу
 * )
 * @var array
 */
/**
 * $map = array(
 *  'type' => section | pad | bssvar
 * 'base' => addr
 * 'size' => int
 * 'value' => Section | (null | char) | Symbol
 * )
 */
$map = array();


echo ".text section starts at 0x", dechex($base_addr), "\n";

$res = build_section($coffs, Section::TYPE_CODE, $base_addr, $section_align);
$map = array_merge($map, $res['map']);

echo ".data section starts at 0x", dechex($res['end_addr']), "\n";
$res = build_section($coffs, Section::TYPE_DATA, $res['end_addr'], $section_align);
$map = array_merge($map, $res['map']);

echo ".bss section starts at 0x", dechex($res['end_addr']), "\n";
$res = build_section($coffs, Section::TYPE_BSS, $res['end_addr'], $section_align);
$map = array_merge($map, $res['map']);

$bss_var_base = $res['end_addr'];
$bss_var_addr = $bss_var_base;
foreach ($coffs as $coff){
	foreach ($coff->symbols as $symbol){
		if ($symbol->SectionNumber == 0 && $symbol->Value > 0 && $symbol->StorageClass > 0){
			
			
			//echo $symbol->Name, "Size ", $symbol->Value , "\n";
			//print_symbol_info($symbol);
			
			$var_addr = $bss_var_addr;
			$var_size = $symbol->Value;
			
			//XXX:// align?
			$pad_size = calc_pad($var_addr, $section_align);
			if ($pad_size > 0){
					$map []= array(
						'type' => 'pad',
						'base' => $var_addr,
						'size' => $pad_size,
						'value' => "\0"
					);
					
					$var_addr += $pad_size;
			}
			
			
			$map []= array(
				'type' => 'bssvar',
				'base' => $var_addr,
				'size' => $var_size,
				'value' => $symbol
			);
			
			$bss_var_addr += $var_size;
			
			echo "BSS ", $symbol->Name, " at 0x", dechex($var_addr)  ,"\n";
		}
	}
}

echo "Linking ...\n";
echo "Linker max adddress 0x",dechex($bss_var_addr),"\n";

$f = fopen($kernel_bin_file, 'w');



foreach($map as $m){
	if ($m['type'] == 'section'){
		$section = $m['value'];
		
		$section_bin = $section->getRawData();
		
		foreach ($section->relocations as $reloc){
				//VirtualAddress
				//SymbolTableIndex
	 			//Type
			$symbol = $section->getSymbol($reloc['SymbolTableIndex']);
			
			if ($symbol->NumberOfAuxSymbols > 0){
				throw new \Exception("NumberOfAuxSymbols > 0. Don't know what to do");
			}
			
			//echo "Resolving: ", $symbol->Name, " " ;
			
			$addr = resolve_symbol_addr($map, $coffs, $symbol, $section);
			if ($addr === null){
				
				echo("Can't resolve \n");
				print_symbol_info($symbol);
				echo "\nObj file: ", $section->coff->file;
				
				exit(1);
			} else {
				//echo "at 0x", dechex($addr) , "\n";
				// relloc!
				
				$reloc_type = $reloc['Type'];
				echo "Realloc ", $symbol->Name,
				" Type: ", dechex($reloc['Type']),
				" reloc_img_addr=0x", dechex(ftell($f) - 1 + $reloc['VirtualAddress']),
				" point_mem_addr=0x", dechex($addr),
				//" point_img_addr=0x", dechex($loader_size + ($addr - $base_addr))
				" point_img_addr=0x", dechex(($addr - $base_addr))
				;
				
								
				$ptr = $reloc['VirtualAddress'];
				
				if ($reloc_type == 0x14){
					// IMAGE_REL_I386_REL32	0x0014	
					// The 32-bit relative displacement of the target. 
					// This supports the x86 relative branch and call instructions.
					
					
					$internal_offset = unpack("loffset", substr($section_bin, $ptr, 4));
					if ($internal_offset['offset'] != 0){
						echo " +offset=", $internal_offset['offset'];
					}
					
					
					$rel_addr = $addr - ($section->baseAddress + $reloc['VirtualAddress'] + 4); // +4 относительно конца адреса
					
					$rel_addr += $internal_offset['offset'];
					
					$addr_bin = pack("l", $rel_addr); //l 	знаковый long (всегда 32 бит, машинный порядок)
					
					echo " base_addr=0x", dechex($section->baseAddress + $reloc['VirtualAddress']);
					echo " rel_add=",  $rel_addr, " hex:0x", dechex($rel_addr);
				} else if ($reloc_type == 0x06){
					// IMAGE_REL_I386_DIR32	0x0006	The target’s 32-bit VA.
					$internal_offset = unpack("Voffset", substr($section_bin, $ptr, 4));
					if ($internal_offset['offset'] != 0){
						echo " +offset=", $internal_offset['offset'];
					}
					$addr += $internal_offset['offset'];
					
					$addr_bin = pack("V", $addr); // V 	беззнаковый long (всегда 32 бит, порядок little endian)
					echo " abs_add=0x",  dechex($addr);
				} else {
					throw new Exception("Unknown reloc type " .  $reloc_type);
				}
				
				echo "\n";
				
				
				for (
						$ptr = $reloc['VirtualAddress'], 
						$i = 0,
						$len = strlen($addr_bin); $i < $len; $i++, $ptr++){
							$section_bin[$ptr] = $addr_bin[$i];
				}
				
				
			}
		}
		
		
		// write section
		fwrite($f, $section_bin);
		
	} else if ($m['type'] == 'pad'){
		$pad_char = ($m['value'] === null ? "\0" : $m['value'][0]); 
		fwrite($f, str_repeat($pad_char, $m['size']));
		
	} else if ($m['type'] == 'bssvar'){
		fwrite($f, str_repeat("\0", $m['size']));
	} else {
		throw new Exception("Unknown object in map");
	}
}

$img_size = ftell($f);
echo "Written ", number_format($img_size), " bytes\n";
fclose($f);







if (!file_exists($loader_file)){
	fwrite(STDERR, "Loader.bin doesn't exist in $loader_file");
	exit(1);
}
// join them together
copy($loader_file, $kedos_file);
file_put_contents($kedos_file, file_get_contents($kernel_bin_file), FILE_APPEND);




echo "After kernel pad\n";
$ksize = filesize($kedos_file);

$pad_to = 512 * 20;
if ($ksize < $pad_to){
	//fwrite($f, str_repeat(chr(0), $pad_to - $kernel_size));
	file_put_contents($kedos_file, str_repeat(chr(0), $pad_to - $ksize), FILE_APPEND);
} else {
	echo "Kernel too big, change loader.asm\n";
}

echo "Done\n";


/**
* ***** CLASSES *****
*/

class Symbol {
	/**
	 *  The Value field indicates the size if the section number is IMAGE_SYM_UNDEFINED(0).
	 *  If the section number is not zero, then the Value field specifies the offset within the section.
	 */
	const COFF_SYM_CLASS_EXTERNAL = 2;
	/**
	 *  Value:
	 *  The offset of the symbol within the section.
	 *  If the Value field is zero, then the symbol represents a section name.
	 */
	const COFF_SYM_CLASS_STATIC = 3;
	
	const COFF_SYM_CLASS_FUNCTION = 101;
	
	public $Name;
	public $Value;
	public $SectionNumber;
	public $Type;
	public $StorageClass;
	public $NumberOfAuxSymbols;
	
	public $BaseType;
	public $ComplexType;
	
	/**
	 * 
	 * @var Coff
	 */
	public $coff;
	
	
	public function __construct(Coff $coff, $Name, $Value, $SectionNumber, $Type, $StorageClass, $NumberOfAuxSymbols){
		$this->coff = $coff;
		$this->Name = $Name;
		$this->Value = $Value;
		$this->SectionNumber = $SectionNumber;
		$this->Type = $Type;
		$this->StorageClass = $StorageClass;
		$this->NumberOfAuxSymbols = $NumberOfAuxSymbols;
		
		$this->BaseType = $Type & 0xFF;
		$this->ComplexType = ($Type >> 8) & 0xFF;
		
	}
	
	public function isStatic(){
		return $this->StorageClass == Symbol::COFF_SYM_CLASS_STATIC;
	}
	
	public function isExternal(){
		return $this->StorageClass == Symbol::COFF_SYM_CLASS_EXTERNAL;
	}
	
	
}

class Section {
	const COFF_SCN_MEM_DISCARDABLE = 0x02000000;
	const COFF_SCN_CNT_CODE	= 0x00000020;
	const COFF_SCN_CNT_INITIALIZED_DATA	= 0x00000040;
	const COFF_SCN_CNT_UNINITIALIZED_DATA =	0x00000080;
	const COFF_SCN_LNK_REMOVE = 0x00000800;
	const COFF_SCN_LNK_INFO = 0x00000200;
	
	const TYPE_CODE = 1;
	const TYPE_DATA = 2;
	const TYPE_BSS = 3;
	const TYPE_UNUSED = 4; 
	
	/**
	 * Keys are:
	 * Name
	 * VirtualSize;
	 * VirtualAddress;
	 * SizeOfRawData;
	 * PointerToRawData;
	 * PointerToRelocations; // This is set to zero for executable images.
	 * PointerToLinenumbers;
	 * NumberOfRelocations;
	 * NumberOfLinenumbers;
	 * Characteristics;
	 * @var array
	 */
	public $header;
	public $coff;
	
	private $sectionNumber;
	
	/**
	 * Keys:
	 * VirtualAddress
	 * SymbolTableIndex
	 * Type
	 * @var array
	 */
	public $relocations = array();
	
	/**
	 * 
	 * @var int Заполняется при линковке
	 */
	public $baseAddress = null;
	
	
	
	public function __construct(Coff $coff, $sectionNumber, $header){
		$this->coff = $coff;
		$this->header = $header;
		$this->sectionNumber = $sectionNumber;
		$this->parseRelocations();
	}

	private function parseRelocations(){
		$relocations_ptr = $this->header['PointerToRelocations'];
		
		for ($i = 0; $i < $this->header['NumberOfRelocations']; $i++){
			$ptr = $relocations_ptr + $i * Coff::RELOCATION_HEADER_SIZE;
			$relocation_bin = substr($this->coff->obj, $ptr, Coff::RELOCATION_HEADER_SIZE);
			$this->relocations []= unpack("VVirtualAddress/VSymbolTableIndex/vType", $relocation_bin);
			
		}		


	}
	
	/**
	 * Определяет куда поместить секцию
	 */
	public function decideType(){
		if ($this->isDiscardable())
			return self::TYPE_UNUSED;
		
		if ($this->isCode())
			return self::TYPE_CODE;
		if ($this->isInitializedData())
			return self::TYPE_DATA;
		if ($this->isUninitializedData())
			return self::TYPE_BSS;
		return self::TYPE_UNUSED; 
	}
	
	public function getSize(){
		return $this->header['SizeOfRawData'];
	}
	
	public function getRawData(){
		return $this->coff->getSectionRawData($this->sectionNumber);
	}
	
	public function getSymbol($index){
		return $this->coff->getSymbol($index);
	}
	
	public function isCode(){
		return ($this->header['Characteristics'] & self::COFF_SCN_CNT_CODE) == self::COFF_SCN_CNT_CODE
		;
	}
	
	public function isInitializedData(){
		return ($this->header['Characteristics'] & self::COFF_SCN_CNT_INITIALIZED_DATA) == self::COFF_SCN_CNT_INITIALIZED_DATA
		;
	}
	
	public function isUninitializedData(){
		return ($this->header['Characteristics'] & self::COFF_SCN_CNT_UNINITIALIZED_DATA) == self::COFF_SCN_CNT_UNINITIALIZED_DATA
		;
	}
	
	public function isDiscardable(){
		$c = $this->header['Characteristics'];
		return 
			(($c & self::COFF_SCN_MEM_DISCARDABLE) == self::COFF_SCN_MEM_DISCARDABLE)
			|| (($c & self::COFF_SCN_LNK_INFO) == self::COFF_SCN_LNK_INFO)
			|| (($c & self::COFF_SCN_LNK_REMOVE) == self::COFF_SCN_LNK_REMOVE)
		;
	}
}

class Coff {
	const HEADER_SIZE = 20;
	const SECTION_HEADER_SIZE = 40;
	const SYMBOL_SIZE = 18;
	const RELOCATION_HEADER_SIZE = 10;
	
	

	public $header;
	public $sections = array();
	
	
	public $symbols = array();
	
	public $obj;
	
	private $string_table_ptr;
	
	public $file;
	
	public function __construct($file){
		$this->file = $file;
		$obj = file_get_contents($file);
		$this->obj = $obj;
		
		$header_bin = substr($obj, 0, 20);
		$this->header = unpack(
				"vMachine/vNumberOfSections/VTimeDateStamp/VPointerToSymbolTable"
				."/VNumberOfSymbols/vSizeOfOptionalHeader/vCharacteristics", $header_bin);
		
		
		if ($this->header['Machine'] != 0x14C){
			// 0x14C - Intel 386 or later processors and compatible processors
			throw new Exception("Unsupported platform");	
		}
		
		$header_end_ptr = self::HEADER_SIZE + $this->header['SizeOfOptionalHeader'];
		
		$this->string_table_ptr = 
			$this->header['PointerToSymbolTable'] + 
			$this->header['NumberOfSymbols'] * self::SYMBOL_SIZE;
		
		
		for ($i = 0; $i < $this->header['NumberOfSections']; $i++){
			$section_head_bin = 
				substr($obj, $header_end_ptr + self::SECTION_HEADER_SIZE * $i, self::SECTION_HEADER_SIZE);
			$section_header = $this->parseSectionHeader($section_head_bin);

			$this->sections []= new Section($this, $i, $section_header);
			
		}
		
		
	
		
		for ($i = 0; $i < $this->header['NumberOfSymbols']; $i++){
			$symbol_bin = substr($obj, 
				$this->header['PointerToSymbolTable'] + $i * self::SYMBOL_SIZE,
				self::SYMBOL_SIZE);
			
			$this->symbols []= $this->parseSymbol($symbol_bin);
				
		}
				
	}	
	
	/**
	 * 
	 * @param int $number zero-based
	 */
	public function getSection($number){
	if (!isset($this->sections[$number])){
			throw new Exception("Section $number doesn't exist");
		}
		return $this->sections[$number];
	}
	/**
	 * 
	 * @param int $number zero-based
	 */
	public function getSectionRawData($number){
		if (!isset($this->sections[$number])){
			throw new Exception("Section $number doesn't exist");
		}
		
		$section = $this->sections[$number];
		
		if ($section->header['PointerToRawData'] == 0)
			return "";
		
		return substr($this->obj, $section->header['PointerToRawData'], $section->header['SizeOfRawData']);		
	}	
	
	public function getSymbol($index){
		if (!isset($this->symbols[$index])){
			throw new Exception("Symbol #$index doesn't exist");
		}
		
		return $this->symbols[$index];
	}
	
	private function parseSymbol($data){
		/*
		  union {
			char Name[8]; // if first 4 bytes are zeros than is's short name
			// so the last 4 bytes are 'An offset into the string table'.
			struct SymbolTableShortName ShortName;
			} Name;
			long Value;
			short SectionNumber; // signed
			short Type;
			char StorageClass;
			char NumberOfAuxSymbols;
			
		 */
		$name = '';
			
		
		if ($data[0] == "\0" && $data[1] == "\0" && $data[2] == "\0" && $data[3] == "\0"){
			// address of name is in rest 4 bytes
			
			$pointer = unpack("VPointer", substr($data, 4, 4));
			
			$name = $this->getStringFromStringTable($pointer['Pointer']);
		} else {
			for	($i = 0; $i < 8; $i++){
				$chr_code = $data[$i];
				if ($chr_code != "\0"){
					$name .= $data[$i];
				} else {
					break;
				}
			}
		}
		
		$symbol = unpack("VValue/sSectionNumber/vType/cStorageClass/cNumberOfAuxSymbols",
			substr($data, 8));
		
		
		return new Symbol($this, 
				$name, 
				$symbol['Value'], 
				$symbol['SectionNumber'],
				$symbol['Type'],
				$symbol['StorageClass'],
				$symbol['NumberOfAuxSymbols']
		);
		
	}
	
	private function getStringFromStringTable($pointer){
		
		$ptr = $this->string_table_ptr + $pointer;
		$name = '';
		while (isset($this->obj[$ptr]) && $this->obj[$ptr] != "\0"){
			$name .= $this->obj[$ptr];
			$ptr++;
		}
		return $name;
	}
	
	private function parseSectionHeader($data){
		/*
		 struct SectionTable {
	char Name[8];
	long VirtualSize;
	long VirtualAddress;
	long SizeOfRawData;
	long PointerToRawData;
	long PointerToRelocations; // This is set to zero for executable images.
	long PointerToLinenumbers;
	short NumberOfRelocations;
	short NumberOfLinenumbers;
	long Characteristics;
		 */
		$section = unpack(
				"c8Name"
				."/VVirtualSize"
				."/VVirtualAddress"
				."/VSizeOfRawData"
				."/VPointerToRawData"
				."/VPointerToRelocations"
				."/VPointerToLinenumbers"
				."/vNumberOfRelocations"
				."/vNumberOfLinenumbers"
				."/VCharacteristics"
				, $data);
		$name = '';
		for ($i = 1; $i <= 8; $i++){
			$char = $section['Name' . $i]; 
			if ($char == 0)
				break;
			$name .= chr($section['Name' . $i]);
		}
		for ($i = 1; $i <= 8; $i++){
			unset($section['Name' . $i]);
		}
		$section['Name'] = $name;
		return $section;
	}
}



/* 
* ******** FUNCTIONS ********
*/


function return_addres_for_section($symbol, $coff){
	$symbol_section = $coff->getSection($symbol->SectionNumber - 1);
	
	if ($symbol_section->baseAddress == null){
		throw new Exception("Symbol points to section that is not in destination image");
	}
	
	return $symbol->Value + $symbol_section->baseAddress;
}

/**
 * 
 * @param array $map
 * @param array $coffs
 * @param Symbol $symbol
 * @param Section $section
 * @throws Exception
 * @return int|null адрес символа
 */
function resolve_symbol_addr($map, $coffs, $symbol, $section){
	
	if ($symbol->SectionNumber > 0){
		return return_addres_for_section($symbol, $section->coff);
	} else if ($symbol->SectionNumber == 0){
		
		if ($symbol->isStatic()){
			// static bss?
			throw new Exception("Whoa! Static BSS. Don't know how to handle it yet");
		} else if ($symbol->isExternal()) {
			
			if ($symbol->Value > 0){
				// external bss ?
				foreach($map as $m){
					if ($m['type'] == 'bssvar'){
						if ($m['value']->Name == $symbol->Name){
							return $m['base'];
						}	
					}
				}
				
			} else {
				foreach ($coffs as $coff){
					foreach ($coff->symbols as $s){
						//echo $s->Name, "\n";
						/*if ($symbol->Name == $s->Name){
							echo "Found match ", "\n";
							print_symbol_info($s);
							echo "\nSame: ", ($s == $symbol ? "true" : "false"), "\n";
							echo "--*-*-*--*-*-\n";
						}
						*/
						if ($s->isExternal() && $s->SectionNumber > 0 && $symbol->Name == $s->Name){
							return return_addres_for_section($s, $coff);
						}
					}
				}
				
			}
			
			
			
			
			
		} else {
			throw new Exception("Symbol not static, not external. Wut?");
		}
		
	} else {
		throw new Exception("Wrong section number " + $symbol->SectionNumber);
	}
	
}


function print_symbol_info(Symbol $symbol){
	echo "Name: ", $symbol->Name,"\n";
	echo "SectionNumber: ", $symbol->SectionNumber,"\n";
	echo sprintf("Type: Base type=%s Complex type=%s", $symbol->Type & 0xFF, ($symbol->Type >> 8) & 0xFF),"\n";
	echo "Value: ", $symbol->Value,"\n";
	echo "StorageClass: ", $symbol->StorageClass,"\n";
}


function build_section($coffs, $sectionType, $base_addr, $section_align){
	$map = array();
	foreach ($coffs as $coff){
		foreach ($coff->sections as $section){
			if ($section->decideType() == $sectionType){
				
				$section_addr = $base_addr;
				
				$base_addr += $section->getSize();
				$pad_base = $base_addr;
				$pad_size = calc_pad($base_addr, $section_align);		
				
				$base_addr += $pad_size;
			
				$map []= array(
					'type' => 'section',
					'base' => $section_addr,
					'size' => $section->getSize(),
					'value' => $section
				);
				
				$section->baseAddress = $section_addr;
				
				$pad_char = $section->isCode() ? "\x90" : "\0"; // 0x90 - nop
				
				if ($pad_size > 0){
					$map []= array(
						'type' => 'pad',
						'base' => $pad_base,
						'size' => $pad_size,
						'value' => $pad_char
					);
				}
				echo "Name: ", $section->header['Name'], " (", basename($coff->file) ,")" , "\n";
				echo "Addr: 0x", dechex($section_addr), "\n";
				echo "Size: ", $section->getSize() ," byte(s)\n";
				echo "Padd: ", $pad_size, " byte(s) char: 0x", dechex($pad_char) ,"\n";
				echo "\n";
				
			}
			
		}
	}
	return array(
		'map' => $map,
		'end_addr' => $base_addr
	);
}

function calc_pad($addr, $align){
	$mod = $addr % $align;
	return $mod == 0 ? 0 : $align - $mod; 
}
