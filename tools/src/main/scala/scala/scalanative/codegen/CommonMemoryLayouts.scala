package scala.scalanative
package codegen

private[codegen] class CommonMemoryLayouts(implicit meta: Metadata) {

  sealed abstract class Layout(types: List[nir.Type]) {
    def this(types: nir.Type*) = this(types.toList)

    val layout: nir.Type.StructValue = nir.Type.StructValue(types.toList)
    def size: Long = MemoryLayout.sizeOf(layout)(meta.platform)
  }

  private object Common {
    final val RttiIdx =
      if (meta.usesPythonAbi) 2
      else 0
    final val LockWordIdx =
      if (meta.usesLockWords) RttiIdx + 1
      else -1
  }

  object Rtti
      extends Layout(
        nir.Type.Ptr :: // ClassRtti
          meta.lockWordType.toList ::: // optional, multithreading only
          nir.Type.Int :: // ClassId
          nir.Type.Int :: // InterfacesCount
          nir.Type.Ptr :: // Interfaces
          nir.Type.Ptr :: // ClassName
          Nil
      ) {
    final val RttiIdx = 0
    final val LockWordIdx =
      if (meta.usesLockWords) RttiIdx + 1
      else -1
    final val ClassIdIdx =
      if (meta.usesLockWords) LockWordIdx + 1
      else RttiIdx + 1
    final val InterfacesCountIdx = ClassIdIdx + 1
    final val InterfacesIdx = InterfacesCountIdx + 1
    final val ClassNameIdx = InterfacesIdx + 1
  }

  // RTTI specific for classes, see class RuntimeTypeInformation
  object ClassRtti extends Layout() {
    val usesDynMap = meta.analysis.dynsigs.nonEmpty
    private val dynMapType = if (usesDynMap) Some(DynamicHashMap.ty) else None
    private val pyTypeCache =
      if (meta.usesPythonAbi) Some(nir.Type.Ptr) else None
    // Common layout not including variable-sized virtual table
    private val baseLayout =
      meta.pythonHeaderType.toList :::
        Rtti.layout ::
        nir.Type.Int :: // class size
        nir.Type.Int :: // id range
        nir.Type.Ptr :: // reference offsets
        // Free slot for additional Int32 to be used in the future
        nir.Type.Int :: // itableSize
        nir.Type.Ptr :: // itables
        nir.Type.Ptr :: // superClass
        pyTypeCache.toList :::
        dynMapType.toList :::
        Nil

    override val layout =
      genLayout(vtable = nir.Type.ArrayValue(nir.Type.Ptr, 0))

    def genLayout(vtable: nir.Type): nir.Type.StructValue =
      nir.Type.StructValue(
        baseLayout ::: vtable :: Nil
      )

    final val RttiIdx = Common.RttiIdx
    final val SizeIdx = RttiIdx + 1
    final val IdRangeIdx = SizeIdx + 1
    final val ReferenceOffsetsIdx = IdRangeIdx + 1
    final val ITableSizeIdx = ReferenceOffsetsIdx + 1
    final val ItablesIdx = ITableSizeIdx + 1
    final val SuperClassIdx = ItablesIdx + 1
    final val PyTypeCacheIdx = if (meta.usesPythonAbi) SuperClassIdx + 1 else -1
    private final val SuperClassMergerIdx =
      if (meta.usesPythonAbi) PyTypeCacheIdx else SuperClassIdx
    final val DynmapIdx = if (usesDynMap) SuperClassMergerIdx + 1 else -1
    final val VtableIdx =
      if (usesDynMap) DynmapIdx + 1 else SuperClassMergerIdx + 1
  }

  object ITable extends Layout() {
    override val layout: nir.Type.StructValue = genLayout(itableSize = 0)
    def genLayout(itableSize: Int) = nir.Type.StructValue(
      nir.Type.Int // id
        :: nir.Type.ArrayValue(nir.Type.Ptr, itableSize)
        :: Nil
    )
  }

  object ObjectHeader
      extends Layout(
        meta.pythonHeaderType.toList :::
          nir.Type.Ptr :: // RTTI
          meta.lockWordType.toList // optional, multithreading only
      ) {
    final val RttiIdx = Common.RttiIdx
    final val LockWordIdx = Common.LockWordIdx
  }

  object Object
      extends Layout(
        ObjectHeader.layout,
        nir.Type.ArrayValue(nir.Type.Ptr, 0)
      ) {
    final val ObjectHeaderIdx = 0
    final val ValuesOffset = ObjectHeaderIdx + 1
  }

  object ArrayHeader
      extends Layout(
        meta.pythonHeaderType.toList :::
          nir.Type.Ptr :: // RTTI
          meta.lockWordType.toList ::: // optional, multithreading only
          nir.Type.Int :: // length
          nir.Type.Int :: // stride (used only by GC)
          Nil
      ) {
    final val PyRefCntIdx = 0
    final val PyTypeIdx = 1
    final val RttiIdx = Common.RttiIdx
    final val LockWordIdx = Common.LockWordIdx
    final val LengthIdx =
      if (meta.usesLockWords) LockWordIdx + 1
      else RttiIdx + 1
    final val StrideIdx = LengthIdx + 1
  }

  object Array
      extends Layout(
        ArrayHeader.layout,
        nir.Type.ArrayValue(nir.Type.Nothing, 0)
      ) {
    final val ArrayHeaderIdx = 0
    final val ValuesIdx = ArrayHeaderIdx + 1
  }

}
