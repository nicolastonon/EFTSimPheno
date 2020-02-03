# xxx

# //--------------------------------------------
from tensorflow.keras.models import Sequential, load_model, model_from_json
from tensorflow.keras.layers import Dense, Dropout, AlphaDropout, Activation, BatchNormalization, LeakyReLU
from tensorflow.keras import regularizers
from tensorflow.keras.regularizers import l2
from tensorflow.keras.callbacks import TensorBoard, EarlyStopping, LambdaCallback, LearningRateScheduler, ReduceLROnPlateau
from tensorflow.keras.utils import plot_model
# from tensorflow.keras.layers.advanced_activations import PReLU




# //--------------------------------------------
# //--------------------------------------------
# //--------------------------------------------
 ######  ########  ########    ###    ######## ########    ##     ##  #######  ########  ######## ##
##    ## ##     ## ##         ## ##      ##    ##          ###   ### ##     ## ##     ## ##       ##
##       ##     ## ##        ##   ##     ##    ##          #### #### ##     ## ##     ## ##       ##
##       ########  ######   ##     ##    ##    ######      ## ### ## ##     ## ##     ## ######   ##
##       ##   ##   ##       #########    ##    ##          ##     ## ##     ## ##     ## ##       ##
##    ## ##    ##  ##       ##     ##    ##    ##          ##     ## ##     ## ##     ## ##       ##
 ######  ##     ## ######## ##     ##    ##    ########    ##     ##  #######  ########  ######## ########
# //--------------------------------------------
# //--------------------------------------------
# //--------------------------------------------

#Advice :
# If your targets are [0,1], use a sigmoid output layer and binary_crossentropy loss.
# If your targets are [-1,1], use a linear or tanh output layer and hinge or squared_hinge loss.
# If your targets are labels for k categories, use to_categorical to convert to one-hot, use a softmax output layer with k outputs, and use categorical_crossentropy loss.

# * Has you model's performance plateaued? If not train for more epochs.
# * Compare the performance on training versus test sample. Are you over training?

#Sigmoid/tanh/softmax work fine for classifiers, but can have problems of vanishing gradients
#ReLu activations should only be used for hidden layers, avoids vanishing gradient issue
#Should use sigmoid (binary) of softmax (multiclass) for output layer, to get class probabilities ? NB : if nof_outputs=1, softmax doesn't seem to work

#Define here the Keras DNN model
def Create_Model(outdir, DNN_name, nof_outputs, var_list):

    use_batchNorm = True
    use_dropout = True #Necessary to avoid overtraining (for deep or wide NN)

    #Set some hyperparameters
    # decay_rate = learning_rate / _nepochs
    droprate = 0.5

    # Define model
    model = Sequential()
    num_input_variables = len(var_list) #Nof input variables to be read by the DNN

    # my_init = 'Zeros' #-- Can check that with this init and few training epochs, ROC is ~0.5 (no time to learn)
    # my_init = 'Ones' #-- Can check that with this init and few training epochs, ROC is ~0.5 (no time to learn)

    my_init = 'he_normal'
    # my_init = 'glorot_normal'
    # my_init = 'lecun_normal'

    #Regularizers
    #NB : "In practice, if you are not concerned with explicit feature selection, L2 regularization can be expected to give superior performance over L1."
    # my_regul = regularizers.l1(0.001)
    my_regul = regularizers.l2(0.001) #Default 0.001
    # my_regul = regularizers.l1_l2(l1=0.01, l2=0.01)

    #Examples of advanced activations (should be added as layers, after dense layers)
    # model.add(LeakyReLU(alpha=0.1))
    # model.add(PReLU(alpha_initializer=my_init))
    # model.add(Activation('selu'))

    model_choice = 1

    #-- List different models, by order of complexity

    #Model 1 -- simple
    if model_choice == 1:
       # //--------------------------------------------
        model.add(Dense(64, kernel_initializer=my_init, activation='tanh', input_dim=num_input_variables)) #, name="myInputs"
        # model.add(Dropout(droprate))
        model.add(Dense(64, kernel_initializer=my_init, activation='relu'))
        # model.add(Dropout(droprate))
        model.add(Dense(64, kernel_initializer=my_init, activation='relu'))
        # model.add(Dropout(droprate))

        if nof_outputs == 1 :
            model.add(Dense(nof_outputs, kernel_initializer=my_init, activation='sigmoid'))
        elif nof_outputs == 2:
            model.add(Dense(nof_outputs, kernel_initializer=my_init, activation='softmax')) #, name="myOutputs"
       # //--------------------------------------------

    #Model 2 -- more elaborate, overtrained
    elif model_choice == 2:
       # //--------------------------------------------
       model.add(Dense(100, input_dim=num_input_variables, activation='tanh', kernel_initializer=my_init) ) #Input layer
       # model.add(BatchNormalization())
       model.add(Dropout(droprate))

       model.add(Dense(100, activation='relu', kernel_initializer=my_init) ) #hidden layer
       # model.add(BatchNormalization())
       # model.add(LeakyReLU(alpha=0.1))
       model.add(Dropout(droprate))

       model.add(Dense(100, activation='relu', kernel_initializer=my_init) ) #hidden layer
       # model.add(BatchNormalization())
       # model.add(LeakyReLU(alpha=0.1))
       model.add(Dropout(droprate))

       model.add(Dense(100, activation='relu', kernel_initializer=my_init) ) #hidden layer
       # model.add(BatchNormalization())
       # model.add(LeakyReLU(alpha=0.1))
       model.add(Dropout(droprate))

       model.add(Dense(nof_outputs, activation='softmax', kernel_initializer=my_init) ) #output layer
       # //--------------------------------------------

    #Model 2 -- more elaborate, overtrained
    elif model_choice == 3:
        # //--------------------------------------------
        model.add(Dense(150, input_dim=num_input_variables, activation='tanh', kernel_initializer=my_init) ) #Input layer
        if use_batchNorm==True:
            model.add(BatchNormalization())
        if use_dropout==True:
            model.add(Dropout(droprate))

        model.add(Dense(150, activation='relu', kernel_initializer=my_init) ) #hidden layer
        model.add(LeakyReLU(alpha=0.1))
        if use_batchNorm==True:
            model.add(BatchNormalization())
        if use_dropout==True:
            model.add(Dropout(droprate))

        model.add(Dense(150, activation='relu', kernel_initializer=my_init) ) #hidden layer
        # model.add(LeakyReLU(alpha=0.1))
        if use_batchNorm==True:
            model.add(BatchNormalization())
        if use_dropout==True:
            model.add(Dropout(droprate))

        model.add(Dense(150, activation='relu', kernel_initializer=my_init) ) #hidden layer
        # model.add(LeakyReLU(alpha=0.1))
        if use_batchNorm==True:
            model.add(BatchNormalization())
        if use_dropout==True:
            model.add(Dropout(droprate))

        model.add(Dense(150, activation='relu', kernel_initializer=my_init) ) #hidden layer
        if use_batchNorm==True:
            model.add(BatchNormalization())
        if use_dropout==True:
            model.add(Dropout(droprate))

        # model.add(Dense(150, activation='relu', kernel_initializer=my_init) ) #hidden layer
        # if use_batchNorm==True:
        #     model.add(BatchNormalization())
        # if use_dropout==True:
        #     model.add(Dropout(droprate))

        model.add(Dense(nof_outputs, activation='softmax', kernel_initializer=my_init) ) #output layer
# //--------------------------------------------


    #Model 3 -- even more elaborate : advanced activation, etc.
    elif model_choice == 4:
        # //--------------------------------------------
        model.add(Dense(150, input_dim=num_input_variables, activation='tanh', kernel_initializer=my_init) ) #Input layer
        # model.add(Dense(100, input_dim=num_input_variables, kernel_regularizer=my_regul, kernel_initializer=my_init) ) #Input layer
        # model.add(PReLU(alpha_initializer=my_init))
        # model.add(Activation('selu'))
        # if use_batchNorm==True:
        #     model.add(BatchNormalization())
        # if use_dropout==True:
        #     model.add(Dropout(droprate))
        # model.add(AlphaDropout(droprate))

        model.add(Dense(150, kernel_initializer=my_init) ) #hidden layer
        # model.add(PReLU(alpha_initializer=my_init))
        model.add(LeakyReLU(alpha=0.1))
        # model.add(Activation('selu'))
        if use_batchNorm==True:
            model.add(BatchNormalization())
        if use_dropout==True:
            model.add(Dropout(droprate))

        model.add(Dense(150, kernel_initializer=my_init) ) #hidden layer
        # model.add(PReLU(alpha_initializer=my_init))
        model.add(LeakyReLU(alpha=0.1))
        # model.add(Activation('selu'))
        if use_batchNorm==True:
            model.add(BatchNormalization())
        if use_dropout==True:
            model.add(Dropout(droprate))

        model.add(Dense(150, kernel_initializer=my_init) ) #hidden layer
        # model.add(PReLU(alpha_initializer=my_init))
        model.add(LeakyReLU(alpha=0.1))
        # model.add(Activation('selu'))
        if use_batchNorm==True:
            model.add(BatchNormalization())
        if use_dropout==True:
            model.add(Dropout(droprate))


        model.add(Dense(100, kernel_initializer=my_init) ) #hidden layer
        # model.add(PReLU(alpha_initializer=my_init))
        model.add(LeakyReLU(alpha=0.1))
        if use_batchNorm==True:
            model.add(BatchNormalization())
        if use_dropout==True:
            model.add(Dropout(droprate))

        model.add(Dense(nof_outputs, activation='softmax', kernel_initializer=my_init) ) #output layer
       # //--------------------------------------------

    else:
        print("\n-- ERROR : wrong model_choice value !\n")

    #Model visualization
    print(model.summary())

    # outname = outdir+'graphviz_'+DNN_name+'.png'
    # plot_model(model, to_file=outname, show_shapes=True, show_layer_names=True, dpi=96)
    # print("\n-- Created DNN arch plot with graphviz : " + outname)

    # outname = outdir+'annviz_'+DNN_name+'.gv'
    # ann_viz(model, title="Neural network architecture", filename=outname, view=True) #cant handle batchnorm?
    # print("\n-- Created DNN arch plot with annviz : " + outname)

    return model
# //--------------------------------------------
# //--------------------------------------------
